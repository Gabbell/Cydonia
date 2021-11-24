#include <Graphics/Vulkan/CommandBuffer.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/PipelineInfos.h>

#include <Graphics/Vulkan/CommandBufferPool.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/PipelineCache.h>
#include <Graphics/Vulkan/DescriptorPool.h>
#include <Graphics/Vulkan/SamplerCache.h>
#include <Graphics/Vulkan/RenderPassCache.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <array>

namespace vk
{

static constexpr uint32_t INITIAL_RESOURCE_TO_UPDATE_COUNT = 32;

CommandBuffer::CommandBuffer()
{
   m_useCount = std::make_unique<std::atomic<uint32_t>>( 0 );

   // TODO Make this static, it is class-wide
   _initStateValidationTable();
}

void CommandBuffer::incUse() { ( *m_useCount )++; }
void CommandBuffer::decUse()
{
   if( m_useCount->load() == 0 )
   {
      CYDASSERT( !"CommandBuffer: Decrementing use count would go below 0" );
      return;
   }

   ( *m_useCount )--;
}

void CommandBuffer::_initStateValidationTable()
{
   // Initialize all valid states [currentState][desiredState]
   m_stateValidationTable[State::ACQUIRED][State::RECORDING]  = true;
   m_stateValidationTable[State::RECORDING][State::STANDBY]   = true;
   m_stateValidationTable[State::STANDBY][State::SUBMITTED]   = true;
   m_stateValidationTable[State::STANDBY][State::SUBMITTED]   = true;
   m_stateValidationTable[State::SUBMITTED][State::COMPLETED] = true;
   m_stateValidationTable[State::COMPLETED][State::FREE]      = true;
   m_stateValidationTable[State::FREE][State::RELEASED]       = true;
   m_stateValidationTable[State::RELEASED][State::ACQUIRED]   = true;
}

bool CommandBuffer::_isValidStateTransition( State desiredState )
{
   if( m_stateValidationTable[m_state][desiredState] )
   {
      m_state = desiredState;
      return true;
   }

   CYDASSERT( !"CommandBuffer: Invalid state transition detected" );

   return false;
}

void CommandBuffer::acquire(
    const Device& device,
    const CommandBufferPool& pool,
    CYD::QueueUsageFlag usage,
    const std::string_view name )
{
   if( _isValidStateTransition( State::ACQUIRED ) )
   {
      m_pDevice = &device;
      m_pPool   = &pool;
      m_usage   = usage;
      m_name    = name;

      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool                 = m_pPool->getVKCommandPool();
      allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount          = 1;

      VkResult result =
          vkAllocateCommandBuffers( m_pDevice->getVKDevice(), &allocInfo, &m_vkCmdBuffer );
      CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Could not allocate command buffer" );

      VkFenceCreateInfo fenceInfo = {};
      fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

      result = vkCreateFence( m_pDevice->getVKDevice(), &fenceInfo, nullptr, &m_vkFence );
      CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create fence" );

      CYDASSERT(
          ( m_semsToSignal.empty() && m_semsToWait.empty() ) &&
          "CommandBuffer: Still have semaphores during acquire" );

      m_semsToSignal.resize( 1 );

      VkSemaphoreCreateInfo semaphoreInfo = {};
      semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      result = vkCreateSemaphore(
          m_pDevice->getVKDevice(), &semaphoreInfo, nullptr, &m_semsToSignal[0] );
      CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create semaphore" );

      m_defaultSampler = m_pDevice->getSamplerCache().findOrCreate( {} );

      m_buffersToUpdate.reserve( INITIAL_RESOURCE_TO_UPDATE_COUNT );
      m_texturesToUpdate.reserve( INITIAL_RESOURCE_TO_UPDATE_COUNT );

      incUse();
   }
}

void CommandBuffer::free()
{
   if( _isValidStateTransition( State::FREE ) )
   {
      m_usage = 0;

      // Clearing tracked descriptor sets
      std::vector<VkDescriptorSet> vkDescSets;
      vkDescSets.reserve( m_descSets.size() );

      for( const auto& descSet : m_descSets )
      {
         vkDescSets.push_back( descSet.vkDescSet );
      }

      if( !vkDescSets.empty() )
      {
         m_pDevice->getDescriptorPool().free(
             vkDescSets.data(), static_cast<uint32_t>( vkDescSets.size() ) );
      }

      // Clearing accumulated framebuffers
      for( const auto& framebuffer : m_curFramebuffers )
      {
         vkDestroyFramebuffer( m_pDevice->getVKDevice(), framebuffer, nullptr );
      }
      m_curFramebuffers.clear();

      m_descSets.clear();

      m_boundPip.reset();
      m_boundPipInfo.reset();
      m_boundPipLayout.reset();
      m_boundRenderPass.reset();

      vkFreeCommandBuffers(
          m_pDevice->getVKDevice(), m_pPool->getVKCommandPool(), 1, &m_vkCmdBuffer );

      m_defaultSampler = nullptr;
      m_vkCmdBuffer    = nullptr;
      m_pPool          = nullptr;

      // This current command buffer is completed so we can free the resources on which it was
      // dependent
      for( CommandBuffer* cmdBuffer : m_cmdBuffersInUse )
      {
         cmdBuffer->decUse();
      }

      for( Texture* texture : m_texturesInUse )
      {
         texture->decUse();
      }

      for( Buffer* buffer : m_buffersInUse )
      {
         buffer->decUse();
      }

      m_cmdBuffersInUse.clear();
      m_texturesInUse.clear();
      m_buffersInUse.clear();
   }
}

void CommandBuffer::release()
{
   if( _isValidStateTransition( State::RELEASED ) )
   {
      vkDestroyFence( m_pDevice->getVKDevice(), m_vkFence, nullptr );
      m_vkFence = nullptr;

      // The semaphore to signal at index 0 is always present and is owned by this command
      // buffer
      vkDestroySemaphore( m_pDevice->getVKDevice(), m_semsToSignal.front(), nullptr );

      m_stagesToWait.clear();
      m_semsToWait.clear();
      m_semsToSignal.clear();

      m_pDevice = nullptr;
   }
}

bool CommandBuffer::isCompleted()
{
   bool isCompleted = ( vkGetFenceStatus( m_pDevice->getVKDevice(), m_vkFence ) == VK_SUCCESS );

   if( isCompleted && _isValidStateTransition( State::COMPLETED ) )
   {
      return isCompleted;
   }

   return isCompleted;
}

void CommandBuffer::waitForCompletion() const
{
   vkWaitForFences( m_pDevice->getVKDevice(), 1, &m_vkFence, VK_TRUE, UINTMAX_MAX );
}

template <class T>
void CommandBuffer::_addDependency( T* dependency )
{
   bool shouldIncrementUsage = false;

   if constexpr( std::is_same_v<T, vk::Buffer> )
   {
      const auto& result   = m_buffersInUse.insert( dependency );
      shouldIncrementUsage = result.second;
   }
   else if constexpr( std::is_same_v<T, vk::Texture> )
   {
      const auto& result   = m_texturesInUse.insert( dependency );
      shouldIncrementUsage = result.second;
   }
   else if constexpr( std::is_same_v<T, vk::CommandBuffer> )
   {
      const auto& result   = m_cmdBuffersInUse.insert( dependency );
      shouldIncrementUsage = result.second;
   }
   else
   {
      static_assert( !"CommandBuffer: Adding dependency of unknown type" );
   }

   if( shouldIncrementUsage ) dependency->incUse();
}

void CommandBuffer::syncOnCommandList( CommandBuffer* cmdBufferToWaitOn )
{
   m_stagesToWait.push_back( cmdBufferToWaitOn->getWaitStages() );
   m_semsToWait.push_back( cmdBufferToWaitOn->getDoneSemaphore() );

   _addDependency( cmdBufferToWaitOn );
}

void CommandBuffer::syncOnSwapchain( const Swapchain* swapchain )
{
   m_stagesToWait.push_back( getWaitStages() );
   m_semsToWait.push_back( swapchain->getSemToWait() );
}

void CommandBuffer::syncToSwapchain( const Swapchain* swapchain )
{
   m_semsToSignal.push_back( swapchain->getSemToSignal() );
}

VkPipelineStageFlags CommandBuffer::getWaitStages() const noexcept
{
   VkPipelineStageFlags waitStages = 0;
   if( m_usage & CYD::QueueUsage::GRAPHICS )
   {
      waitStages |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
   }
   if( m_usage & CYD::QueueUsage::TRANSFER )
   {
      waitStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
   }
   if( m_usage & CYD::QueueUsage::COMPUTE )
   {
      waitStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   }

   return waitStages;
}

VkSemaphore CommandBuffer::getDoneSemaphore() const
{
   if( !m_semsToSignal.empty() )
   {
      return m_semsToSignal.front();
   }

   return VK_NULL_HANDLE;
}

void CommandBuffer::startRecording()
{
   if( _isValidStateTransition( State::RECORDING ) )
   {
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      const VkResult result = vkBeginCommandBuffer( m_vkCmdBuffer, &beginInfo );
      CYDASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to begin recording of command buffer" );
   }
}

void CommandBuffer::endRecording()
{
   if( _isValidStateTransition( State::STANDBY ) )
   {
      const VkResult result = vkEndCommandBuffer( m_vkCmdBuffer );
      CYDASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to end recording of command buffer" );

      m_boundPip        = std::nullopt;
      m_boundPipLayout  = std::nullopt;
      m_boundRenderPass = std::nullopt;

      m_boundPipInfo.reset();
   }
}

void CommandBuffer::reset()
{
   if( _isValidStateTransition( State::ACQUIRED ) )
   {
      vkResetCommandBuffer( m_vkCmdBuffer, {} );

      // Clearing tracked descriptor sets
      for( const auto& descSetInfo : m_descSets )
      {
         m_pDevice->getDescriptorPool().free( descSetInfo.vkDescSet );
      }

      m_descSets.clear();
   }
}

void CommandBuffer::updatePushConstants( const CYD::PushConstantRange& range, const void* pData )
{
   CYDASSERT( m_boundPipLayout.has_value() && "CommandBuffer: No currently bound pipeline layout" );

   vkCmdPushConstants(
       m_vkCmdBuffer,
       m_boundPipLayout.value(),
       TypeConversions::cydToVkShaderStages( range.stages ),
       static_cast<uint32_t>( range.offset ),
       static_cast<uint32_t>( range.size ),
       pData );
}

void CommandBuffer::bindPipeline( const CYD::GraphicsPipelineInfo& info )
{
   CYDASSERT(
       m_boundRenderPass.has_value() &&
       "CommandBuffer: Cannot bind pipeline because not in a render pass" );

   VkPipeline pipeline =
       m_pDevice->getPipelineCache().findOrCreate( info, m_boundRenderPass.value() );

   VkPipelineLayout pipLayout = m_pDevice->getPipelineCache().findOrCreate( info.pipLayout );

   CYDASSERT(
       pipeline && pipLayout &&
       "CommandBuffer: Could not find or create pipeline in pipeline stash" );

   vkCmdBindPipeline( m_vkCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
   m_boundPip       = pipeline;
   m_boundPipLayout = pipLayout;
   m_boundPipInfo   = std::make_unique<CYD::GraphicsPipelineInfo>( info );
}

void CommandBuffer::bindPipeline( const CYD::ComputePipelineInfo& info )
{
   VkPipeline pipeline = m_pDevice->getPipelineCache().findOrCreate( info );

   VkPipelineLayout pipLayout = m_pDevice->getPipelineCache().findOrCreate( info.pipLayout );

   CYDASSERT(
       pipeline && pipLayout &&
       "CommandBuffer: Could not find or create pipeline in pipeline stash" );

   vkCmdBindPipeline( m_vkCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline );
   m_boundPip       = pipeline;
   m_boundPipLayout = pipLayout;
   m_boundPipInfo   = std::make_unique<CYD::ComputePipelineInfo>( info );
}

void CommandBuffer::bindVertexBuffer( Buffer* vertexBuffer )
{
   VkBuffer vertexBuffers[] = { vertexBuffer->getVKBuffer() };
   VkDeviceSize offsets[]   = { 0 };
   vkCmdBindVertexBuffers( m_vkCmdBuffer, 0, 1, vertexBuffers, offsets );

   _addDependency( vertexBuffer );
}

void CommandBuffer::bindIndexBuffer( Buffer* indexBuffer, CYD::IndexType type )
{
   vkCmdBindIndexBuffer(
       m_vkCmdBuffer, indexBuffer->getVKBuffer(), 0, TypeConversions::cydToVkIndexType( type ) );

   _addDependency( indexBuffer );
}

void CommandBuffer::bindTexture( Texture* texture, uint32_t set, uint32_t binding )
{
   // Will need to update this texture's descriptor set before next draw
   m_texturesToUpdate.emplace_back(
       texture, CYD::ShaderResourceType::COMBINED_IMAGE_SAMPLER, set, binding );

   _addDependency( texture );
}

void CommandBuffer::bindImage( Texture* texture, uint32_t set, uint32_t binding )
{
   // Will need to update this image descriptor set before next draw
   m_texturesToUpdate.emplace_back( texture, CYD::ShaderResourceType::STORAGE_IMAGE, set, binding );

   // TODO Eventually we will need more info when binding an image (level for mipmaps for example)
   _addDependency( texture );
}

void CommandBuffer::bindBuffer( Buffer* buffer, uint32_t set, uint32_t binding )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back( buffer, CYD::ShaderResourceType::STORAGE, set, binding );

   _addDependency( buffer );
}

void CommandBuffer::bindUniformBuffer( Buffer* buffer, uint32_t set, uint32_t binding )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back( buffer, CYD::ShaderResourceType::UNIFORM, set, binding );

   _addDependency( buffer );
}

void CommandBuffer::setViewport( const CYD::Viewport& viewport ) const
{
   VkViewport vkViewport = {
       viewport.offsetX,
       viewport.offsetY,
       viewport.width,
       viewport.height,
       viewport.minDepth,
       viewport.maxDepth };
   vkCmdSetViewport( m_vkCmdBuffer, 0, 1, &vkViewport );
}

void CommandBuffer::setScissor( const CYD::Rectangle& scissor ) const
{
   VkRect2D vkScissor = {
       scissor.offset.x, scissor.offset.y, scissor.extent.width, scissor.extent.height };
   vkCmdSetScissor( m_vkCmdBuffer, 0, 1, &vkScissor );
}

void CommandBuffer::beginRendering( Swapchain& swapchain )
{
   VkRenderPass renderPass = swapchain.getRenderPass();
   CYDASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass = renderPass;

   VkRenderPassBeginInfo passBeginInfo = {};
   passBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   passBeginInfo.renderPass            = m_boundRenderPass.value();
   passBeginInfo.framebuffer           = swapchain.getCurrentFramebuffer();
   passBeginInfo.renderArea.offset     = { 0, 0 };
   passBeginInfo.renderArea.extent     = swapchain.getVKExtent();

   std::array<VkClearValue, 2> clearValues = {};
   clearValues[0]                          = { 0.2f, 0.2f, 0.2f, 1.0f };
   clearValues[1]                          = { 1.0f, 0 };

   passBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
   passBeginInfo.pClearValues    = clearValues.data();

   vkCmdBeginRenderPass( m_vkCmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

   // We want the next renders in this swapchain to load the previous result
   swapchain.setToLoad();
}

void CommandBuffer::beginRendering(
    const CYD::RenderTargetsInfo& targetsInfo,
    const std::vector<const Texture*>& targets )
{
   VkRenderPass renderPass = m_pDevice->getRenderPassCache().findOrCreate( targetsInfo );
   CYDASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass  = renderPass;
   m_boundTargetsInfo = targetsInfo;
   m_currentSubpass   = 0;

   const uint32_t commonWidth  = targets[0]->getWidth();
   const uint32_t commonHeight = targets[0]->getHeight();

   // Fetching image views for framebuffer
   std::vector<VkImageView> vkImageViews;
   vkImageViews.reserve( targets.size() );
   for( const auto& texture : targets )
   {
      // All textures should have the same dimensions for this renderpass/framebuffer
      if( texture->getWidth() == commonWidth && texture->getHeight() == commonHeight )
      {
         vkImageViews.push_back( texture->getVKImageView() );
      }
      else
      {
         CYDASSERT( !"CommandBuffer: Mismatched dimensions for framebuffer" );
      }
   }

   VkFramebufferCreateInfo framebufferInfo = {};
   framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferInfo.renderPass              = m_boundRenderPass.value();
   framebufferInfo.attachmentCount         = static_cast<uint32_t>( vkImageViews.size() );
   framebufferInfo.pAttachments            = vkImageViews.data();
   framebufferInfo.width                   = commonWidth;
   framebufferInfo.height                  = commonHeight;
   framebufferInfo.layers                  = 1;

   VkFramebuffer vkFramebuffer;
   const VkResult result =
       vkCreateFramebuffer( m_pDevice->getVKDevice(), &framebufferInfo, nullptr, &vkFramebuffer );

   CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create framebuffer" );

   m_curFramebuffers.push_back( vkFramebuffer );

   VkRenderPassBeginInfo passBeginInfo = {};
   passBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   passBeginInfo.renderPass            = m_boundRenderPass.value();
   passBeginInfo.framebuffer           = vkFramebuffer;
   passBeginInfo.renderArea.offset     = { 0, 0 };
   passBeginInfo.renderArea.extent     = { commonWidth, commonHeight };

   std::array<VkClearValue, 2> clearValues = {};
   clearValues[0]                          = { 0.0f, 0.0f, 0.0f, 1.0f };
   clearValues[1]                          = { 1.0f, 0.0f };

   passBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
   passBeginInfo.pClearValues    = clearValues.data();

   vkCmdBeginRenderPass( m_vkCmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

VkDescriptorSet CommandBuffer::_findOrAllocateDescSet( size_t prevSize, uint32_t set )
{
   const auto it = std::find_if(
       m_descSets.begin() + prevSize, m_descSets.end(), [set]( const DescriptorSetInfo& info ) {
          return info.set == set;
       } );

   if( it != m_descSets.end() )
   {
      // This set has already been created during this draw scope, just return it
      return it->vkDescSet;
   }

   // Adding this descriptor set to the list of all tracked descriptor set in this command buffer
   const VkDescriptorSet vkDescSet =
       m_pDevice->getDescriptorPool().allocate( m_boundPipInfo->pipLayout.shaderSets[set] );

   m_descSets.push_back( { set, vkDescSet } );

   m_setsToBind[set] = vkDescSet;
   return vkDescSet;
}

void CommandBuffer::_prepareDescriptorSets( CYD::PipelineType pipType )
{
   // We do not care about any of the descriptor sets before this index
   const size_t prevSize = m_descSets.size();

   // Creating write descriptors for resources we want to update for this draw
   const size_t totalSize = m_buffersToUpdate.size() + m_texturesToUpdate.size();

   // New descriptor sets that need to be allocated for this draw call
   std::vector<VkDescriptorBufferInfo> bufferInfos;
   std::vector<VkDescriptorImageInfo> imageInfos;
   std::vector<VkWriteDescriptorSet> writeDescSets;
   bufferInfos.reserve( totalSize );
   imageInfos.reserve( totalSize );
   writeDescSets.reserve( totalSize );

   for( const auto& entry : m_buffersToUpdate )
   {
      VkDescriptorBufferInfo bufferInfo;
      bufferInfo.buffer = entry.resource->getVKBuffer();
      bufferInfo.offset = 0;
      bufferInfo.range  = entry.resource->getSize();
      bufferInfos.push_back( bufferInfo );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet               = _findOrAllocateDescSet( prevSize, entry.set );
      descriptorWrite.dstBinding           = entry.binding;
      descriptorWrite.dstArrayElement      = 0;
      descriptorWrite.descriptorType       = TypeConversions::cydToVkDescriptorType( entry.type );
      descriptorWrite.descriptorCount      = 1;
      descriptorWrite.pBufferInfo          = &bufferInfos.back();

      writeDescSets.push_back( descriptorWrite );
   }

   for( const auto& entry : m_texturesToUpdate )
   {
      VkDescriptorImageInfo imageInfo = {};

      // Determine layout based on descriptor type
      imageInfo.imageLayout = TypeConversions::cydToVkImageLayout( entry.resource->getLayout() );
      imageInfo.imageView   = entry.resource->getVKImageView();
      imageInfo.sampler     = m_defaultSampler;
      imageInfos.push_back( imageInfo );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet               = _findOrAllocateDescSet( prevSize, entry.set );
      descriptorWrite.dstBinding           = entry.binding;
      descriptorWrite.dstArrayElement      = 0;
      descriptorWrite.descriptorType       = TypeConversions::cydToVkDescriptorType( entry.type );
      descriptorWrite.descriptorCount      = 1;
      descriptorWrite.pImageInfo           = &imageInfos.back();

      writeDescSets.push_back( descriptorWrite );
   }

   // Updating the descriptor sets for this draw
   vkUpdateDescriptorSets(
       m_pDevice->getVKDevice(),
       static_cast<uint32_t>( writeDescSets.size() ),
       writeDescSets.data(),
       0,
       nullptr );

   // Binding the descriptor sets we want for this draw (expensive apparently)
   VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   switch( pipType )
   {
      case CYD::PipelineType::GRAPHICS:
         bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
         break;
      case CYD::PipelineType::COMPUTE:
         bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
         break;
      default:
         CYDASSERT( !"CommandBuffer: Could not determine pipeline bind point for descriptors" );
   }

   // Determine how to bind descriptor sets in the case of noncontinuous sets. Usually, this is a
   // bad idea and should never happen but hey, you can do it.
   uint32_t contiguousCount = 0;

   for( int i = 0; i < m_setsToBind.size(); ++i )
   {
      if( m_setsToBind[i] != nullptr )
      {
         contiguousCount++;
      }

      if( ( m_setsToBind[i] == nullptr || i == ( m_setsToBind.size() - 1 ) ) &&
          contiguousCount > 0 )
      {
         const uint32_t startIdx = ( i - contiguousCount );

         vkCmdBindDescriptorSets(
             m_vkCmdBuffer,
             bindPoint,
             m_boundPipLayout.value(),
             startIdx,
             contiguousCount,
             &m_setsToBind[startIdx],
             0,
             nullptr );

         contiguousCount = 0;
      }
   }

   m_setsToBind.fill( nullptr );
   m_buffersToUpdate.clear();
   m_texturesToUpdate.clear();
}

void CommandBuffer::draw( size_t vertexCount, size_t firstVertex )
{
   CYDASSERT(
       m_usage & CYD::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command buffer does not support graphics usage" );

   CYDASSERT(
       m_boundPip && m_boundPipInfo && ( m_boundPipInfo->type == CYD::PipelineType::GRAPHICS ) &&
       "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets( CYD::PipelineType::GRAPHICS );

   vkCmdDraw(
       m_vkCmdBuffer,
       static_cast<uint32_t>( vertexCount ),
       1,
       static_cast<uint32_t>( firstVertex ),
       0 );
}

void CommandBuffer::drawIndexed( size_t indexCount, size_t firstIndex )
{
   CYDASSERT(
       m_usage & CYD::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command Buffer does not support graphics usage" );

   CYDASSERT(
       m_boundPip && m_boundPipInfo && ( m_boundPipInfo->type == CYD::PipelineType::GRAPHICS ) &&
       "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets( CYD::PipelineType::GRAPHICS );

   vkCmdDrawIndexed(
       m_vkCmdBuffer,
       static_cast<uint32_t>( indexCount ),
       1,
       static_cast<uint32_t>( firstIndex ),
       0,
       0 );
}

void CommandBuffer::dispatch( uint32_t workX, uint32_t workY, uint32_t workZ )
{
   CYDASSERT(
       m_usage & CYD::QueueUsage::COMPUTE &&
       "CommandBuffer: Command Buffer does not support compute usage" );

   CYDASSERT(
       m_boundPip && m_boundPipInfo && m_boundPipInfo->type == CYD::PipelineType::COMPUTE &&
       "CommandBuffer: Cannot dispatch because no proper pipeline was bound" );

   _prepareDescriptorSets( CYD::PipelineType::COMPUTE );

   vkCmdDispatch( m_vkCmdBuffer, workX, workY, workZ );
}

void CommandBuffer::nextPass() const
{
   CYDASSERT(
       m_boundRenderPass.has_value() &&
       "CommandBuffer: Cannot go to the next pass if there was no render pass to begin with!" );

   vkCmdNextSubpass( m_vkCmdBuffer, VK_SUBPASS_CONTENTS_INLINE );
}

void CommandBuffer::endRendering() const
{
   CYDASSERT(
       m_boundRenderPass.has_value() &&
       "CommandBuffer: Cannot end a pass that was never started!" );

   vkCmdEndRenderPass( m_vkCmdBuffer );
}

void CommandBuffer::copyBuffer( Buffer* src, Buffer* dst )
{
   CYDASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   VkBufferCopy copyRegion = {};
   copyRegion.size         = dst->getSize();
   vkCmdCopyBuffer( m_vkCmdBuffer, src->getVKBuffer(), dst->getVKBuffer(), 1, &copyRegion );

   _addDependency( src );
   _addDependency( dst );
}

void CommandBuffer::uploadBufferToTex( Buffer* src, Texture* dst )
{
   CYDASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   // Copying data from buffer to texture
   VkBufferImageCopy region               = {};
   region.bufferOffset                    = 0;
   region.bufferRowLength                 = 0;
   region.bufferImageHeight               = 0;
   region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   region.imageSubresource.mipLevel       = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount     = dst->getLayers();
   region.imageOffset                     = { 0, 0, 0 };
   region.imageExtent                     = { dst->getWidth(), dst->getHeight(), 1 };

   vkCmdCopyBufferToImage(
       m_vkCmdBuffer,
       src->getVKBuffer(),
       dst->getVKImage(),
       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
       1,
       &region );

   _addDependency( src );
   _addDependency( dst );
}

void CommandBuffer::submit()
{
   if( _isValidStateTransition( State::SUBMITTED ) )
   {
      VkSubmitInfo submitInfo       = {};
      submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers    = &m_vkCmdBuffer;

      submitInfo.waitSemaphoreCount   = static_cast<uint32_t>( m_semsToWait.size() );
      submitInfo.pWaitSemaphores      = m_semsToWait.data();
      submitInfo.pWaitDstStageMask    = m_stagesToWait.data();
      submitInfo.signalSemaphoreCount = static_cast<uint32_t>( m_semsToSignal.size() );
      submitInfo.pSignalSemaphores    = m_semsToSignal.data();

      const Device::QueueFamily& queueFamily =
          m_pDevice->getQueueFamilyFromIndex( m_pPool->getFamilyIndex() );

      CYDASSERT(
          !queueFamily.queues.empty() && "CommandBuffer: Could not find queue to submit to" );

      // TODO Dynamic submission to multiple queues
      vkQueueSubmit( queueFamily.queues[0], 1, &submitInfo, m_vkFence );
   }
}

CommandBuffer::~CommandBuffer()
{
   if( isFree() )
   {
      release();
   }
}
}
