#include <Graphics/Vulkan/CommandBuffer.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/Framebuffer.h>

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
#include <Graphics/Vulkan/Synchronization.h>

#include <array>

namespace vk
{
static constexpr uint32_t INITIAL_RESOURCE_TO_UPDATE_COUNT = 32;

CommandBuffer::CommandBuffer() { m_useCount = std::make_unique<std::atomic<uint32_t>>( 0 ); }

void CommandBuffer::incUse() { ( *m_useCount )++; }
void CommandBuffer::decUse()
{
   if( m_useCount->load() == 0 )
   {
      CYD_ASSERT( !"CommandBuffer: Decrementing use count would go below 0" );
      return;
   }

   ( *m_useCount )--;
}

bool CommandBuffer::_isValidStateTransition( State desiredState )
{
   // Initialize all valid states [currentState][desiredState]
   static bool stateValidationTable[NUMBER_OF_STATES][NUMBER_OF_STATES] = {};
   stateValidationTable[State::ACQUIRED][State::ACQUIRED]               = true;
   stateValidationTable[State::ACQUIRED][State::RECORDING]              = true;
   stateValidationTable[State::RECORDING][State::STANDBY]               = true;
   stateValidationTable[State::STANDBY][State::SUBMITTED]               = true;
   stateValidationTable[State::SUBMITTED][State::COMPLETED]             = true;
   stateValidationTable[State::COMPLETED][State::FREE]                  = true;
   stateValidationTable[State::FREE][State::RELEASED]                   = true;
   stateValidationTable[State::RELEASED][State::ACQUIRED]               = true;

   if( stateValidationTable[m_state][desiredState] )
   {
      m_state = desiredState;
      return true;
   }

   CYD_ASSERT( !"CommandBuffer: Invalid state transition detected" );

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
      CYD_ASSERT( result == VK_SUCCESS && "CommandBuffer: Could not allocate command buffer" );

      VkFenceCreateInfo fenceInfo = {};
      fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

      result = vkCreateFence( m_pDevice->getVKDevice(), &fenceInfo, nullptr, &m_vkFence );
      CYD_ASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create fence" );

      CYD_ASSERT(
          ( m_semsToSignal.empty() && m_semsToWait.empty() ) &&
          "CommandBuffer: Still have semaphores during acquire" );

      m_semsToSignal.resize( 1 );

      VkSemaphoreCreateInfo semaphoreInfo = {};
      semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      result = vkCreateSemaphore(
          m_pDevice->getVKDevice(), &semaphoreInfo, nullptr, &m_semsToSignal[0] );
      CYD_ASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create semaphore" );

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
      for( const auto& descSet : m_descSets )
      {
         m_pDevice->getDescriptorPool().free( descSet.vkDescSet );
      }

      // Clearing accumulated framebuffers
      for( const auto& framebuffer : m_curFramebuffers )
      {
         vkDestroyFramebuffer( m_pDevice->getVKDevice(), framebuffer, nullptr );
      }
      m_curFramebuffers.clear();

      m_descSets.clear();

      m_boundPip        = nullptr;
      m_boundPipLayout  = nullptr;
      m_boundRenderPass = nullptr;

      m_boundPipInfo.reset();

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
      m_name    = DEFAULT_CMDBUFFER_NAME;
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
   vkWaitForFences( m_pDevice->getVKDevice(), 1, &m_vkFence, VK_TRUE, UINT64_MAX );
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
      static_assert(
          std::is_same_v<T, vk::Buffer> || std::is_same_v<T, vk::Texture> ||
              std::is_same_v<T, vk::CommandBuffer>,
          "CommandBuffer: Adding dependency of unknown type" );
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
      CYD_ASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to begin recording of command buffer" );
   }
}

void CommandBuffer::endRecording()
{
   if( _isValidStateTransition( State::STANDBY ) )
   {
      const VkResult result = vkEndCommandBuffer( m_vkCmdBuffer );
      CYD_ASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to end recording of command buffer" );
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
   CYD_ASSERT( m_boundPipLayout && "CommandBuffer: No currently bound pipeline layout" );

   vkCmdPushConstants(
       m_vkCmdBuffer,
       m_boundPipLayout,
       TypeConversions::cydToVkShaderStages( range.stages ),
       static_cast<uint32_t>( range.offset ),
       static_cast<uint32_t>( range.size ),
       pData );
}

static bool samePipeline( const CYD::PipelineInfo* lhs, const CYD::PipelineInfo* rhs )
{
   if( lhs == nullptr || rhs == nullptr ) return false;
   if( lhs->type != rhs->type ) return false;

   switch( lhs->type )
   {
      case CYD::PipelineType::GRAPHICS:
         return *static_cast<const CYD::GraphicsPipelineInfo*>( lhs ) ==
                *static_cast<const CYD::GraphicsPipelineInfo*>( rhs );

      case CYD::PipelineType::COMPUTE:
         return *static_cast<const CYD::ComputePipelineInfo*>( lhs ) ==
                *static_cast<const CYD::ComputePipelineInfo*>( rhs );
   }

   return false;
}

void CommandBuffer::bindPipeline( const CYD::GraphicsPipelineInfo& info )
{
   CYD_ASSERT(
       m_boundRenderPass && "CommandBuffer: Cannot bind pipeline because not in a render pass" );

   if( samePipeline( &info, m_boundPipInfo.get() ) ) return;

   VkPipeline pipeline = m_pDevice->getPipelineCache().findOrCreate(
       info, m_boundRenderPassInfo.value(), m_boundRenderPass );

   VkPipelineLayout pipLayout = m_pDevice->getPipelineCache().findOrCreate( info.pipLayout );

   CYD_ASSERT(
       pipeline && pipLayout &&
       "CommandBuffer: Could not find or create pipeline in pipeline stash" );

   vkCmdBindPipeline( m_vkCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
   m_boundPip       = pipeline;
   m_boundPipLayout = pipLayout;
   m_boundPipInfo   = std::make_unique<CYD::GraphicsPipelineInfo>( info );
}

void CommandBuffer::bindPipeline( const CYD::ComputePipelineInfo& info )
{
   if( samePipeline( &info, m_boundPipInfo.get() ) ) return;

   VkPipeline pipeline = m_pDevice->getPipelineCache().findOrCreate( info );

   VkPipelineLayout pipLayout = m_pDevice->getPipelineCache().findOrCreate( info.pipLayout );

   CYD_ASSERT(
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

void CommandBuffer::bindIndexBuffer( Buffer* indexBuffer, CYD::IndexType type, uint32_t offset )
{
   vkCmdBindIndexBuffer(
       m_vkCmdBuffer,
       indexBuffer->getVKBuffer(),
       offset,
       TypeConversions::cydToVkIndexType( type ) );

   _addDependency( indexBuffer );
}

void CommandBuffer::bindTexture( Texture* texture, uint8_t binding, uint8_t set )
{
   if( m_boundPipInfo->type == CYD::PipelineType::COMPUTE )
   {
      Synchronization::ImageMemory( this, texture, CYD::Access::COMPUTE_SHADER_READ );
   }

   // Will need to update this texture's descriptor set before next draw
   TextureBinding& textureEntry = m_texturesToUpdate.emplace_back();
   textureEntry.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   textureEntry.imageView       = texture->getVKImageView();
   textureEntry.binding         = binding;
   textureEntry.set             = set;
   textureEntry.layout = Synchronization::GetLayoutFromAccess( texture->getPreviousAccess() );

   m_samplers.push_back( m_defaultSampler );

   _addDependency( texture );
}

void CommandBuffer::bindTexture(
    Texture* texture,
    const CYD::SamplerInfo& sampler,
    uint8_t binding,
    uint8_t set )
{
   if( m_boundPipInfo->type == CYD::PipelineType::COMPUTE )
   {
      Synchronization::ImageMemory( this, texture, CYD::Access::COMPUTE_SHADER_READ );
   }

   // Will need to update this texture's descriptor set before next draw
   TextureBinding& textureEntry = m_texturesToUpdate.emplace_back();
   textureEntry.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   textureEntry.imageView       = texture->getVKImageView();
   textureEntry.binding         = binding;
   textureEntry.set             = set;
   textureEntry.layout = Synchronization::GetLayoutFromAccess( texture->getPreviousAccess() );

   m_samplers.push_back( m_pDevice->getSamplerCache().findOrCreate( sampler ) );

   _addDependency( texture );
}

void CommandBuffer::bindImage( Texture* texture, uint8_t binding, uint8_t set )
{
   if( m_boundPipInfo->type == CYD::PipelineType::COMPUTE )
   {
      // TODO Maybe not GENERAL?
      Synchronization::ImageMemory( this, texture, CYD::Access::GENERAL );
   }

   // Will need to update this image descriptor set before next draw
   TextureBinding& textureEntry = m_texturesToUpdate.emplace_back();
   textureEntry.type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   textureEntry.imageView       = texture->getVKImageView();
   textureEntry.binding         = binding;
   textureEntry.set             = set;
   textureEntry.layout = Synchronization::GetLayoutFromAccess( texture->getPreviousAccess() );

   m_samplers.push_back( m_defaultSampler );

   // TODO Eventually we will need more info when binding an image (level for mipmaps for example)
   _addDependency( texture );
}

void CommandBuffer::bindBuffer(
    Buffer* buffer,
    uint8_t binding,
    uint8_t set,
    uint32_t offset,
    uint32_t range )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back(
       buffer, CYD::ShaderResourceType::STORAGE, binding, set, offset, range );

   _addDependency( buffer );
}

void CommandBuffer::bindUniformBuffer(
    Buffer* buffer,
    uint8_t binding,
    uint8_t set,
    uint32_t offset,
    uint32_t range )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back(
       buffer, CYD::ShaderResourceType::UNIFORM, binding, set, offset, range );

   _addDependency( buffer );
}

void CommandBuffer::setViewport( const CYD::Viewport& viewport ) const
{
   VkViewport vkViewport;
   if( viewport.width == 0 || viewport.height == 0 )
   {
      CYD_ASSERT(
          m_renderArea.extent.width > 0 && m_renderArea.extent.height > 0 &&
          "CommandBuffer:: Could not automatically determine viewport size" );

      // Default viewport
      vkViewport = {
          static_cast<float>( m_renderArea.offset.x ),
          static_cast<float>( m_renderArea.offset.y ),
          static_cast<float>( m_renderArea.extent.width ),
          static_cast<float>( m_renderArea.extent.height ),
          0.0f,
          1.0f };
   }
   else
   {
      vkViewport = {
          viewport.offsetX,
          viewport.offsetY,
          viewport.width,
          viewport.height,
          viewport.minDepth,
          viewport.maxDepth };
   }

   vkCmdSetViewport( m_vkCmdBuffer, 0, 1, &vkViewport );
}

void CommandBuffer::setScissor( const CYD::Rectangle& scissor ) const
{
   VkRect2D vkScissor;
   if( scissor.extent.width == 0 || scissor.extent.height == 0 )
   {
      CYD_ASSERT(
          m_renderArea.extent.width > 0 && m_renderArea.extent.height > 0 &&
          "CommandBuffer:: Could not automatically determine scissor size" );

      // Default scissor
      vkScissor = {
          m_renderArea.offset.x,
          m_renderArea.offset.y,
          m_renderArea.extent.width,
          m_renderArea.extent.height };
   }
   else
   {
      vkScissor = {
          scissor.offset.x, scissor.offset.y, scissor.extent.width, scissor.extent.height };
   }

   vkCmdSetScissor( m_vkCmdBuffer, 0, 1, &vkScissor );
}

void CommandBuffer::beginRendering( Swapchain& swapchain )
{
   VkRenderPass renderPass = swapchain.getVKRenderPass();
   CYD_ASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass     = renderPass;
   m_boundRenderPassInfo = swapchain.getRenderPass();

   const VkExtent2D& extent = swapchain.getVKExtent();

   VkRenderPassBeginInfo passBeginInfo = {};
   passBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   passBeginInfo.renderPass            = m_boundRenderPass;
   passBeginInfo.framebuffer           = swapchain.getCurrentVKFramebuffer();
   passBeginInfo.renderArea.offset     = { 0, 0 };
   passBeginInfo.renderArea.extent     = extent;

   std::array<VkClearValue, 2> clearValues = {};
   clearValues[0]                          = { 0.0f, 0.0f, 0.0f };  // Color
   clearValues[1]                          = { 0.0f, 0 };           // Depth/Stencil

   passBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
   passBeginInfo.pClearValues    = clearValues.data();

   _setRenderArea( 0, 0, extent.width, extent.height );

   vkCmdBeginRenderPass( m_vkCmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void CommandBuffer::beginRendering(
    const CYD::Framebuffer& fb,
    const std::vector<Texture*>& texTargets )
{
   CYD_ASSERT( !texTargets.empty() && "CommandBuffer: No render targets" );

   const uint32_t fbWidth  = fb.getWidth();
   const uint32_t fbHeight = fb.getHeight();

   const CYD::Framebuffer::RenderTargets& renderTargets = fb.getRenderTargets();

   // Fetching image views for framebuffer
   RenderPassInfo renderPassInfo;
   std::vector<VkClearValue> clearValues;
   std::vector<VkImageView> vkImageViews;
   vkImageViews.reserve( m_targets.size() );
   for( uint32_t i = 0; i < texTargets.size(); ++i )
   {
      Texture* texture = texTargets[i];

      if( texture == nullptr ) continue;

      // All textures should have the same dimensions for this renderpass/framebuffer
      if( texture->getWidth() == fbWidth && texture->getHeight() == fbHeight )
      {
         vkImageViews.push_back( texture->getVKImageView() );
      }
      else
      {
         CYD_ASSERT( !"CommandBuffer: Mismatched dimensions for framebuffer" );
      }

      const CYD::Framebuffer::RenderTarget& rt = renderTargets[i];

      // Infer attachment
      Attachment& attachment   = renderPassInfo.attachments.emplace_back();
      attachment.format        = texture->getPixelFormat();
      attachment.initialAccess = texture->getPreviousAccess();
      attachment.nextAccess    = rt.nextAccess;
      attachment.loadOp        = rt.shouldClear ? CYD::LoadOp::CLEAR : CYD::LoadOp::LOAD;
      attachment.storeOp       = CYD::StoreOp::STORE;
      attachment.clear         = rt.clearValue;

      if( attachment.format == CYD::PixelFormat::D32_SFLOAT )
      {
         attachment.type = CYD::AttachmentType::DEPTH_STENCIL;

         // Depth
         VkClearValue clearValue;
         clearValue.depthStencil = {
             attachment.clear.depthStencil.depth, attachment.clear.depthStencil.stencil };
         clearValues.push_back( std::move( clearValue ) );
      }
      else
      {
         attachment.type = CYD::AttachmentType::COLOR;

         // Assume color
         VkClearValue clearValue;
         memcpy( &clearValue.color, &attachment.clear.color, sizeof( clearValue.color ) );
         clearValues.push_back( std::move( clearValue ) );
      }
   }

   VkRenderPass renderPass = m_pDevice->getRenderPassCache().findOrCreate( renderPassInfo );
   CYD_ASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass     = renderPass;
   m_boundRenderPassInfo = renderPassInfo;
   m_targets             = texTargets;
   m_currentSubpass      = 0;

   VkFramebufferCreateInfo framebufferInfo = {};
   framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferInfo.renderPass              = m_boundRenderPass;
   framebufferInfo.attachmentCount         = static_cast<uint32_t>( vkImageViews.size() );
   framebufferInfo.pAttachments            = vkImageViews.data();
   framebufferInfo.width                   = fbWidth;
   framebufferInfo.height                  = fbHeight;
   framebufferInfo.layers                  = 1;

   VkFramebuffer vkFramebuffer;
   const VkResult result =
       vkCreateFramebuffer( m_pDevice->getVKDevice(), &framebufferInfo, nullptr, &vkFramebuffer );

   CYD_ASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create framebuffer" );

   m_curFramebuffers.push_back( vkFramebuffer );

   VkRenderPassBeginInfo passBeginInfo = {};
   passBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   passBeginInfo.renderPass            = m_boundRenderPass;
   passBeginInfo.framebuffer           = vkFramebuffer;
   passBeginInfo.renderArea.offset     = { 0, 0 };
   passBeginInfo.renderArea.extent     = { fbWidth, fbHeight };

   passBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
   passBeginInfo.pClearValues    = clearValues.data();

   _setRenderArea( 0, 0, fbWidth, fbHeight );

   vkCmdBeginRenderPass( m_vkCmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void CommandBuffer::_setRenderArea( int offsetX, int offsetY, uint32_t width, uint32_t height )
{
   m_renderArea = { { offsetX, offsetY }, { width, height } };
}

VkDescriptorSet CommandBuffer::_findOrAllocateDescSet( size_t prevSize, uint8_t set )
{
   const auto it = std::find_if(
       m_descSets.begin() + prevSize,
       m_descSets.end(),
       [set]( const DescriptorSetInfo& info ) { return info.set == set; } );

   if( it != m_descSets.end() )
   {
      // This set has already been created during this draw scope, just return it
      return it->vkDescSet;
   }

   // Adding this descriptor set to the list of all tracked descriptor set in this command buffer
   const VkDescriptorSet vkDescSet =
       m_pDevice->getDescriptorPool().allocate( m_boundPipInfo->pipLayout.shaderSets[set] );

   m_descSets.emplace_back( set, vkDescSet );

   m_setsToBind[set] = vkDescSet;
   return vkDescSet;
}

void CommandBuffer::_prepareDescriptorSets()
{
   // We do not care about any of the descriptor sets before this index
   const size_t prevSize = m_descSets.size();

   // Creating write descriptors for resources we want to update for this draw
   const size_t totalSize = m_buffersToUpdate.size() + m_texturesToUpdate.size();

   if( totalSize == 0 )
   {
      // Nothing to update
      return;
   }

   // New descriptor sets that need to be updated or allocated for this draw call
   std::vector<VkDescriptorBufferInfo> bufferInfos;
   std::vector<VkDescriptorImageInfo> imageInfos;
   std::vector<VkWriteDescriptorSet> writeDescSets;

   bufferInfos.reserve( totalSize );
   imageInfos.reserve( totalSize );
   writeDescSets.reserve( totalSize );

   for( const BufferBinding& entry : m_buffersToUpdate )
   {
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer                 = entry.resource->getVKBuffer();
      bufferInfo.offset                 = entry.offset;
      bufferInfo.range                  = entry.range > 0 ? entry.range : entry.resource->getSize();
      bufferInfos.push_back( bufferInfo );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet               = _findOrAllocateDescSet( prevSize, entry.set );
      descriptorWrite.dstBinding           = entry.binding;
      descriptorWrite.dstArrayElement      = 0;
      descriptorWrite.descriptorType       = TypeConversions::cydToVkDescriptorType( entry.type );
      descriptorWrite.descriptorCount      = 1;
      descriptorWrite.pBufferInfo          = &bufferInfos.back();

      writeDescSets.push_back( std::move( descriptorWrite ) );
   }

   CYD_ASSERT( m_texturesToUpdate.size() == m_samplers.size() );
   for( uint32_t i = 0; i < m_texturesToUpdate.size(); ++i )
   {
      const TextureBinding& entry = m_texturesToUpdate[i];
      VkSampler sampler           = m_samplers[i];

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.sampler               = sampler;
      imageInfo.imageView             = entry.imageView;
      imageInfo.imageLayout           = entry.layout;
      imageInfos.push_back( imageInfo );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet               = _findOrAllocateDescSet( prevSize, entry.set );
      descriptorWrite.dstBinding           = entry.binding;
      descriptorWrite.dstArrayElement      = 0;
      descriptorWrite.descriptorType       = entry.type;
      descriptorWrite.descriptorCount      = 1;
      descriptorWrite.pImageInfo           = &imageInfos.back();

      writeDescSets.push_back( std::move( descriptorWrite ) );
   }

   // TODO Sanitize m_boundPipInfo vs m_descSetInfos

   // Updating the descriptor sets for this draw
   vkUpdateDescriptorSets(
       m_pDevice->getVKDevice(),
       static_cast<uint32_t>( writeDescSets.size() ),
       writeDescSets.data(),
       0,
       nullptr );

   // Binding the descriptor sets we want for this draw (expensive apparently)
   VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   switch( m_boundPipInfo->type )
   {
      case CYD::PipelineType::GRAPHICS:
         bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
         break;
      case CYD::PipelineType::COMPUTE:
         bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
         break;
      default:
         CYD_ASSERT( !"CommandBuffer: Could not determine pipeline bind point for descriptors" );
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
             m_boundPipLayout,
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
   m_samplers.clear();
}

void CommandBuffer::draw(
    size_t vertexCount,
    size_t instanceCount,
    size_t firstVertex,
    size_t firstInstance )
{
   CYD_ASSERT(
       m_usage & CYD::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command buffer does not support graphics usage" );

   CYD_ASSERT(
       m_boundPip && m_boundPipInfo && ( m_boundPipInfo->type == CYD::PipelineType::GRAPHICS ) &&
       "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets();

   vkCmdDraw(
       m_vkCmdBuffer,
       static_cast<uint32_t>( vertexCount ),
       static_cast<uint32_t>( instanceCount ),
       static_cast<uint32_t>( firstVertex ),
       static_cast<uint32_t>( firstInstance ) );
}

void CommandBuffer::drawIndexed(
    size_t indexCount,
    size_t instanceCount,
    size_t firstIndex,
    size_t firstInstance )
{
   CYD_ASSERT(
       m_usage & CYD::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command Buffer does not support graphics usage" );

   CYD_ASSERT(
       m_boundPip && m_boundPipInfo && ( m_boundPipInfo->type == CYD::PipelineType::GRAPHICS ) &&
       "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets();

   vkCmdDrawIndexed(
       m_vkCmdBuffer,
       static_cast<uint32_t>( indexCount ),
       static_cast<uint32_t>( instanceCount ),
       static_cast<uint32_t>( firstIndex ),
       0,
       static_cast<uint32_t>( firstInstance ) );
}

void CommandBuffer::copyToSwapchain( Texture* sourceTexture, Swapchain& swapchain ) const
{
   // TODO Use vkCmdCopyImage instead
   Synchronization::ImageMemory( this, sourceTexture, CYD::Access::TRANSFER_READ );
   swapchain.transitionColorImage( this, CYD::Access::TRANSFER_WRITE );

   const VkExtent2D& swapchainExtent = swapchain.getVKExtent();

   VkImageCopy region;
   region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   region.srcSubresource.layerCount     = 1;
   region.srcSubresource.baseArrayLayer = 0;
   region.srcSubresource.mipLevel       = 0;
   region.srcOffset                     = { 0, 0, 0 };

   region.dstSubresource = region.srcSubresource;
   region.dstOffset      = { 0, 0, 0 };
   region.extent         = { sourceTexture->getWidth(), sourceTexture->getHeight(), 1 };

   vkCmdCopyImage(
       m_vkCmdBuffer,
       sourceTexture->getVKImage(),
       Synchronization::GetLayoutFromAccess( sourceTexture->getPreviousAccess() ),
       swapchain.getColorVKImage(),
       Synchronization::GetLayoutFromAccess( swapchain.getColorVKImageAccess() ),
       1,
       &region );

   swapchain.transitionColorImage( this, CYD::Access::PRESENT );
}

void CommandBuffer::dispatch( uint32_t workX, uint32_t workY, uint32_t workZ )
{
   CYD_ASSERT(
       m_usage & CYD::QueueUsage::COMPUTE &&
       "CommandBuffer: Command Buffer does not support compute usage" );

   CYD_ASSERT(
       m_boundPip && m_boundPipInfo && m_boundPipInfo->type == CYD::PipelineType::COMPUTE &&
       "CommandBuffer: Cannot dispatch because no proper pipeline was bound" );

   _prepareDescriptorSets();

   // Not ideal, could just synchronize individual images. This makes sequences of dispatch calls
   // keep the proper order in terms of memory read/write
   Synchronization::GlobalMemory(
       m_vkCmdBuffer, CYD::Access::COMPUTE_SHADER_WRITE, CYD::Access::COMPUTE_SHADER_READ );

   vkCmdDispatch( m_vkCmdBuffer, workX, workY, workZ );
}

void CommandBuffer::nextPass() const
{
   CYD_ASSERT(
       m_boundRenderPass &&
       "CommandBuffer: Cannot go to the next pass if there was no render pass to begin with!" );

   vkCmdNextSubpass( m_vkCmdBuffer, VK_SUBPASS_CONTENTS_INLINE );
}

void CommandBuffer::endRendering()
{
   CYD_ASSERT( m_boundRenderPass && "CommandBuffer: Cannot end a pass that was never started!" );

   vkCmdEndRenderPass( m_vkCmdBuffer );

   for( uint32_t i = 0; i < m_targets.size(); ++i )
   {
      Texture* texture = m_targets[i];
      texture->setPreviousAccess( m_boundRenderPassInfo->attachments[i].nextAccess );
   }

   m_boundPip        = nullptr;
   m_boundPipLayout  = nullptr;
   m_boundRenderPass = nullptr;
   m_boundPipInfo.reset();

   m_targets.clear();
   m_renderArea = {};
}

void CommandBuffer::copyBuffer( Buffer* src, Buffer* dst, const CYD::BufferCopyInfo& info )
{
   CYD_ASSERT(
       ( info.srcOffset + info.size ) <= dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   VkBufferCopy copyRegion = {};
   copyRegion.srcOffset    = info.srcOffset;
   copyRegion.dstOffset    = info.dstOffset;
   copyRegion.size         = info.size;

   vkCmdCopyBuffer( m_vkCmdBuffer, src->getVKBuffer(), dst->getVKBuffer(), 1, &copyRegion );

   _addDependency( src );
   _addDependency( dst );
}

void CommandBuffer::copyBufferToTexture(
    Buffer* src,
    Texture* dst,
    const CYD::BufferToTextureInfo& info )
{
   CYD_ASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   Synchronization::ImageMemory( this, dst, CYD::Access::TRANSFER_WRITE );

   // Copying data from buffer to texture
   VkBufferImageCopy region               = {};
   region.bufferOffset                    = info.srcOffset;
   region.bufferRowLength                 = 0;
   region.bufferImageHeight               = 0;
   region.imageSubresource.aspectMask     = TypeConversions::getAspectMask( dst->getPixelFormat() );
   region.imageSubresource.mipLevel       = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount     = dst->getLayers();
   region.imageOffset                     = { info.dstOffset.x, info.dstOffset.y, 0 };
   region.imageExtent                     = { info.dstExtent.width, info.dstExtent.height, 1 };

   vkCmdCopyBufferToImage(
       m_vkCmdBuffer,
       src->getVKBuffer(),
       dst->getVKImage(),
       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
       1,
       &region );

   Synchronization::ImageMemory( this, dst, CYD::Access::FRAGMENT_SHADER_READ );

   _addDependency( src );
   _addDependency( dst );
}

void CommandBuffer::copyTexture( Texture* src, Texture* dst, const CYD::TextureCopyInfo& info )
{
   VkImageCopy imageCopy;
   imageCopy.srcOffset                 = { info.srcOffset.x, info.srcOffset.y, 0 };
   imageCopy.srcSubresource.aspectMask = TypeConversions::getAspectMask( src->getPixelFormat() );
   imageCopy.srcSubresource.baseArrayLayer = 0;
   imageCopy.srcSubresource.layerCount     = 1;
   imageCopy.srcSubresource.mipLevel       = 0;
   imageCopy.dstOffset                     = { info.dstOffset.x, info.dstOffset.y, 0 };
   imageCopy.dstSubresource.aspectMask = TypeConversions::getAspectMask( dst->getPixelFormat() );
   imageCopy.srcSubresource.baseArrayLayer = 0;
   imageCopy.srcSubresource.layerCount     = 1;
   imageCopy.srcSubresource.mipLevel       = 0;
   imageCopy.extent                        = { info.extent.width, info.extent.height, 1 };

   vkCmdCopyImage(
       m_vkCmdBuffer,
       src->getVKImage(),
       Synchronization::GetLayoutFromAccess( src->getPreviousAccess() ),
       dst->getVKImage(),
       Synchronization::GetLayoutFromAccess( dst->getPreviousAccess() ),
       1,
       &imageCopy );
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

      CYD_ASSERT(
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
