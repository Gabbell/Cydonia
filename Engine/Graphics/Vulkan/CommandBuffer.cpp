#include <Graphics/Vulkan/CommandBuffer.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/PipelineInfos.h>

#include <Graphics/Vulkan/CommandPool.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/PipelineStash.h>
#include <Graphics/Vulkan/DescriptorPool.h>
#include <Graphics/Vulkan/SamplerStash.h>
#include <Graphics/Vulkan/RenderPassStash.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <array>

namespace vk
{
static constexpr uint32_t INITIAL_RESOURCE_TO_UPDATE_COUNT = 32;

void CommandBuffer::acquire(
    const Device& device,
    const CommandPool& pool,
    CYD::QueueUsageFlag usage )
{
   m_pDevice = &device;
   m_pPool   = &pool;
   m_usage   = usage;

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

   m_defaultSampler = m_pDevice->getSamplerStash().findOrCreate( {} );

   m_buffersToUpdate.reserve( INITIAL_RESOURCE_TO_UPDATE_COUNT );
   m_texturesToUpdate.reserve( INITIAL_RESOURCE_TO_UPDATE_COUNT );
}

void CommandBuffer::release()
{
   if( m_pDevice )
   {
      m_usage        = 0;
      m_isRecording  = false;
      m_wasSubmitted = false;

      // Clearing tracked descriptor sets
      std::vector<VkDescriptorSet> vkDescSets;
      vkDescSets.reserve( m_descSets.size() );

      for( const auto& descSet : m_descSets )
      {
         vkDescSets.push_back( descSet.vkDescSet );
      }

      m_pDevice->getDescriptorPool().free(
          vkDescSets.data(), static_cast<uint32_t>( vkDescSets.size() ) );

      // Clearing accumulated framebuffers
      for( const auto& framebuffer : m_curFramebuffers )
      {
         vkDestroyFramebuffer( m_pDevice->getVKDevice(), framebuffer, nullptr );
      }
      m_curFramebuffers.clear();

      m_descSets.clear();

      m_semsToWait.clear();
      m_semsToSignal.clear();

      m_boundPip.reset();
      m_boundPipInfo.reset();
      m_boundPipLayout.reset();
      m_boundRenderPass.reset();

      vkDestroyFence( m_pDevice->getVKDevice(), m_vkFence, nullptr );
      vkFreeCommandBuffers(
          m_pDevice->getVKDevice(), m_pPool->getVKCommandPool(), 1, &m_vkCmdBuffer );

      m_defaultSampler = nullptr;
      m_vkCmdBuffer    = nullptr;
      m_vkFence        = nullptr;
      m_pDevice        = nullptr;
      m_pPool          = nullptr;
   }
}

bool CommandBuffer::isCompleted() const
{
   return vkGetFenceStatus( m_pDevice->getVKDevice(), m_vkFence ) == VK_SUCCESS;
}

void CommandBuffer::waitForCompletion() const
{
   vkWaitForFences( m_pDevice->getVKDevice(), 1, &m_vkFence, VK_TRUE, UINTMAX_MAX );
}

void CommandBuffer::startRecording()
{
   CYDASSERT( !m_isRecording && "CommandBuffer: Already started recording" );

   VkCommandBufferBeginInfo beginInfo = {};
   beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

   const VkResult result = vkBeginCommandBuffer( m_vkCmdBuffer, &beginInfo );
   CYDASSERT(
       result == VK_SUCCESS && "CommandBuffer: Failed to begin recording of command buffer" );
   m_isRecording = true;
}

void CommandBuffer::endRecording()
{
   CYDASSERT(
       m_isRecording && "CommandBuffer: Trying to stop recording but was not in recording state" );

   const VkResult result = vkEndCommandBuffer( m_vkCmdBuffer );
   CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Failed to end recording of command buffer" );

   m_boundPip        = std::nullopt;
   m_boundPipLayout  = std::nullopt;
   m_boundRenderPass = std::nullopt;

   m_boundPipInfo.reset();

   m_isRecording = false;
}

void CommandBuffer::reset()
{
   CYDASSERT(
       !m_isRecording && "CommandBuffer: Cannot reset, command buffer is in recording state" );

   vkResetCommandBuffer( m_vkCmdBuffer, {} );

   // Clearing tracked descriptor sets
   for( const auto& descSetInfo : m_descSets )
   {
      m_pDevice->getDescriptorPool().free( descSetInfo.vkDescSet );
   }

   m_descSets.clear();
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
       m_pDevice->getPipelineStash().findOrCreate( info, m_boundRenderPass.value() );

   VkPipelineLayout pipLayout = m_pDevice->getPipelineStash().findOrCreate( info.pipLayout );

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
   VkPipeline pipeline = m_pDevice->getPipelineStash().findOrCreate( info );

   VkPipelineLayout pipLayout = m_pDevice->getPipelineStash().findOrCreate( info.pipLayout );

   CYDASSERT(
       pipeline && pipLayout &&
       "CommandBuffer: Could not find or create pipeline in pipeline stash" );

   vkCmdBindPipeline( m_vkCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline );
   m_boundPip       = pipeline;
   m_boundPipLayout = pipLayout;
   m_boundPipInfo   = std::make_unique<CYD::ComputePipelineInfo>( info );
}

void CommandBuffer::bindVertexBuffer( Buffer* vertexBuffer ) const
{
   VkBuffer vertexBuffers[] = { vertexBuffer->getVKBuffer() };
   VkDeviceSize offsets[]   = { 0 };
   vkCmdBindVertexBuffers( m_vkCmdBuffer, 0, 1, vertexBuffers, offsets );

   vertexBuffer->incUse();
}

void CommandBuffer::bindIndexBuffer( Buffer* indexBuffer, CYD::IndexType type ) const
{
   vkCmdBindIndexBuffer(
       m_vkCmdBuffer, indexBuffer->getVKBuffer(), 0, TypeConversions::cydToVkIndexType( type ) );

   indexBuffer->incUse();
}

void CommandBuffer::bindBuffer( Buffer* buffer, uint32_t set, uint32_t binding )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back( buffer, CYD::ShaderResourceType::STORAGE, set, binding );

   buffer->incUse();
}

void CommandBuffer::bindUniformBuffer( Buffer* buffer, uint32_t set, uint32_t binding )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back( buffer, CYD::ShaderResourceType::UNIFORM, set, binding );

   buffer->incUse();
}

void CommandBuffer::bindTexture( Texture* texture, uint32_t set, uint32_t binding )
{
   // Will need to update this texture's descriptor set before next draw
   m_texturesToUpdate.emplace_back(
       texture, CYD::ShaderResourceType::COMBINED_IMAGE_SAMPLER, set, binding );

   texture->incUse();
}

void CommandBuffer::bindImage( Texture* texture, uint32_t set, uint32_t binding )
{
   // Will need to update this image descriptor set before next draw
   m_texturesToUpdate.emplace_back( texture, CYD::ShaderResourceType::STORAGE_IMAGE, set, binding );

   // TODO Eventually we will need more info when binding an image (level for mipmaps for example)
   texture->incUse();
}

void CommandBuffer::setViewport( const CYD::Viewport& viewport ) const
{
   VkViewport vkViewport = {viewport.offsetX,
                            viewport.offsetY,
                            viewport.width,
                            viewport.height,
                            viewport.minDepth,
                            viewport.maxDepth};
   vkCmdSetViewport( m_vkCmdBuffer, 0, 1, &vkViewport );
}

void CommandBuffer::setScissor( const CYD::Rectangle& scissor ) const
{
   VkRect2D vkScissor = {
       scissor.offset.x, scissor.offset.y, scissor.extent.width, scissor.extent.height};
   vkCmdSetScissor( m_vkCmdBuffer, 0, 1, &vkScissor );
}

void CommandBuffer::beginPass( Swapchain& swapchain, bool hasDepth )
{
   swapchain.initFramebuffers( hasDepth );
   swapchain.acquireImage();

   VkRenderPass renderPass = swapchain.getCurrentRenderPass();
   CYDASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass = renderPass;

   m_semsToWait.push_back( swapchain.getSemToWait() );
   m_semsToSignal.push_back( swapchain.getSemToSignal() );

   VkRenderPassBeginInfo passBeginInfo = {};
   passBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   passBeginInfo.renderPass            = m_boundRenderPass.value();
   passBeginInfo.framebuffer           = swapchain.getCurrentFramebuffer();
   passBeginInfo.renderArea.offset     = { 0, 0 };
   passBeginInfo.renderArea.extent     = swapchain.getVKExtent();

   std::array<VkClearValue, 2> clearValues = {};
   clearValues[0]                          = { 0.0f, 0.0f, 0.0f, 1.0f };
   clearValues[1]                          = { 1.0f, 0 };

   passBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
   passBeginInfo.pClearValues    = clearValues.data();

   vkCmdBeginRenderPass( m_vkCmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void CommandBuffer::beginPass(
    const CYD::RenderPassInfo& renderPassInfo,
    const std::vector<const Texture*>& textures )
{
   VkRenderPass renderPass = m_pDevice->getRenderPassStash().findOrCreate( renderPassInfo );
   CYDASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass = renderPass;

   const uint32_t commonWidth  = textures[0]->getWidth();
   const uint32_t commonHeight = textures[0]->getHeight();

   // Fetching image views for framebuffer
   std::vector<VkImageView> vkImageViews;
   vkImageViews.reserve( textures.size() );
   for( const auto& texture : textures )
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
   framebufferInfo.renderPass              = renderPass;
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
   clearValues[1]                          = { 1.0f, 0 };

   passBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
   passBeginInfo.pClearValues    = clearValues.data();

   vkCmdBeginRenderPass( m_vkCmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

VkDescriptorSet CommandBuffer::_findOrAllocateDescSet( size_t prevSize, uint32_t set )
{
   for( auto it = m_descSets.begin() + prevSize; it != m_descSets.end(); ++it )
   {
      if( it->set == set )
      {
         // This set has already been created during this draw scope, just return it
         return it->vkDescSet;
      }
   }

   // Adding this descriptor set to the list of all tracked descriptor set in this command buffer
   const VkDescriptorSet vkDescSet =
       m_pDevice->getDescriptorPool().allocate( m_boundPipInfo->pipLayout.descSets[set] );

   m_descSets.push_back( { set, vkDescSet } );

   m_boundSets[set] = vkDescSet;
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
      bufferInfo.buffer = entry.buffer->getVKBuffer();
      bufferInfo.offset = 0;
      bufferInfo.range  = entry.buffer->getSize();
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
      switch( entry.type )
      {
         case CYD::ShaderResourceType::COMBINED_IMAGE_SAMPLER:
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
         default:  // Includes images which need to be in general layout for load/store operations
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
      }

      imageInfo.imageView = entry.texture->getVKImageView();
      imageInfo.sampler   = m_defaultSampler;
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

   vkCmdBindDescriptorSets(
       m_vkCmdBuffer,
       bindPoint,
       m_boundPipLayout.value(),
       0,
       static_cast<uint32_t>( m_boundPipInfo->pipLayout.descSets.size() ),  // Must be this
       m_boundSets.data(),
       0,
       nullptr );

   m_buffersToUpdate.clear();
   m_texturesToUpdate.clear();
}

void CommandBuffer::draw( size_t vertexCount )
{
   CYDASSERT(
       m_usage & CYD::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command buffer does not support graphics usage" );

   CYDASSERT(
       m_boundPip && m_boundPipInfo && ( m_boundPipInfo->type == CYD::PipelineType::GRAPHICS ) &&
       "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets( CYD::PipelineType::GRAPHICS );

   vkCmdDraw( m_vkCmdBuffer, static_cast<uint32_t>( vertexCount ), 1, 0, 0 );
}

void CommandBuffer::drawIndexed( size_t indexCount )
{
   CYDASSERT(
       m_usage & CYD::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command Buffer does not support graphics usage" );

   CYDASSERT(
       m_boundPip && m_boundPipInfo && ( m_boundPipInfo->type == CYD::PipelineType::GRAPHICS ) &&
       "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets( CYD::PipelineType::GRAPHICS );

   vkCmdDrawIndexed( m_vkCmdBuffer, static_cast<uint32_t>( indexCount ), 1, 0, 0, 0 );
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

void CommandBuffer::endPass() const
{
   CYDASSERT(
       m_boundRenderPass.has_value() &&
       "CommandBuffer: Cannot end a pass that was never started!" );

   vkCmdEndRenderPass( m_vkCmdBuffer );
}

void CommandBuffer::copyBuffer( const Buffer* src, const Buffer* dst ) const
{
   CYDASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   VkBufferCopy copyRegion = {};
   copyRegion.size         = dst->getSize();
   vkCmdCopyBuffer( m_vkCmdBuffer, src->getVKBuffer(), dst->getVKBuffer(), 1, &copyRegion );
}

void CommandBuffer::uploadBufferToTex( const Buffer* src, Texture* dst ) const
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
   region.imageOffset                     = {0, 0, 0};
   region.imageExtent                     = {dst->getWidth(), dst->getHeight(), 1};

   vkCmdCopyBufferToImage(
       m_vkCmdBuffer,
       src->getVKBuffer(),
       dst->getVKImage(),
       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
       1,
       &region );
}

void CommandBuffer::submit()
{
   VkSubmitInfo submitInfo       = {};
   submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers    = &m_vkCmdBuffer;

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

   submitInfo.waitSemaphoreCount   = static_cast<uint32_t>( m_semsToWait.size() );
   submitInfo.pWaitSemaphores      = m_semsToWait.data();
   submitInfo.pWaitDstStageMask    = &waitStages;
   submitInfo.signalSemaphoreCount = static_cast<uint32_t>( m_semsToSignal.size() );
   submitInfo.pSignalSemaphores    = m_semsToSignal.data();

   const VkQueue* queue = m_pDevice->getQueueFromFamily( m_pPool->getFamilyIndex() );
   CYDASSERT( queue && "CommandBuffer: Could not find queue to submit to" );

   vkQueueSubmit( *queue, 1, &submitInfo, m_vkFence );
   m_wasSubmitted = true;

   m_semsToWait.clear();
   m_semsToSignal.clear();
}
}
