#include <Graphics/Vulkan/CommandBuffer.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/CommandPool.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/PipelineStash.h>
#include <Graphics/Vulkan/RenderPassStash.h>
#include <Graphics/Vulkan/DescriptorPool.h>
#include <Graphics/Vulkan/SamplerStash.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <array>

namespace vk
{
void CommandBuffer::acquire(
    const Device& device,
    const CommandPool& pool,
    cyd::QueueUsageFlag usage )
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
}

void CommandBuffer::release()
{
   if( m_pDevice )
   {
      m_usage        = 0;
      m_isRecording  = false;
      m_wasSubmitted = false;

      // Clearing tracked descriptor sets
      for( const auto& descSet : m_descSets )
      {
         m_pDevice->getDescriptorPool().free( descSet );
      }
      m_descSets.clear();

      m_semsToWait.clear();
      m_semsToSignal.clear();

      m_boundPip.reset();
      m_boundPipInfo.reset();
      m_boundPipLayout.reset();
      m_boundRenderPass.reset();

      m_prevLayout = cyd::ImageLayout::UNDEFINED;

      vkDestroyFence( m_pDevice->getVKDevice(), m_vkFence, nullptr );
      vkFreeCommandBuffers(
          m_pDevice->getVKDevice(), m_pPool->getVKCommandPool(), 1, &m_vkCmdBuffer );

      m_vkCmdBuffer = nullptr;
      m_vkFence     = nullptr;
      m_pDevice     = nullptr;
      m_pPool       = nullptr;
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
   m_boundPipInfo    = std::nullopt;
   m_isRecording     = false;
}

void CommandBuffer::reset()
{
   CYDASSERT(
       !m_isRecording && "CommandBuffer: Cannot reset, command buffer is in recording state" );

   vkResetCommandBuffer( m_vkCmdBuffer, {} );

   // Clearing tracked descriptor sets
   for( const auto& descSet : m_descSets )
   {
      m_pDevice->getDescriptorPool().free( descSet );
   }
   m_descSets.clear();
}

void CommandBuffer::updatePushConstants( const cyd::PushConstantRange& range, const void* pData )
{
   CYDASSERT( m_boundPipLayout.has_value() && "CommandBuffer: No currently bound pipeline layout" );

   vkCmdPushConstants(
       m_vkCmdBuffer,
       m_boundPipLayout.value(),
       TypeConversions::cydShaderStagesToVkShaderStages( range.stages ),
       static_cast<uint32_t>( range.offset ),
       static_cast<uint32_t>( range.size ),
       pData );
}

void CommandBuffer::bindPipeline( const cyd::PipelineInfo& info )
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
   m_boundPipInfo   = info;
}

void CommandBuffer::bindVertexBuffer( const Buffer* vertexBuffer ) const
{
   VkBuffer vertexBuffers[] = { vertexBuffer->getVKBuffer() };
   VkDeviceSize offsets[]   = { 0 };
   vkCmdBindVertexBuffers( m_vkCmdBuffer, 0, 1, vertexBuffers, offsets );
}

template <>
void CommandBuffer::bindIndexBuffer<uint16_t>( const Buffer* indexBuffer )
{
   vkCmdBindIndexBuffer( m_vkCmdBuffer, indexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT16 );
}

template <>
void CommandBuffer::bindIndexBuffer<uint32_t>( const Buffer* indexBuffer )
{
   vkCmdBindIndexBuffer( m_vkCmdBuffer, indexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32 );
}

void CommandBuffer::bindBuffer( const Buffer* buffer, uint32_t set, uint32_t binding )
{
   // Will need to update this buffer's descriptor set before next draw
   m_buffersToUpdate.emplace_back( buffer, set, binding );
}

void CommandBuffer::bindTexture( const Texture* texture, uint32_t set, uint32_t binding )
{
   // Will need to update this texture's descriptor set before next draw
   m_texturesToUpdate.emplace_back( texture, set, binding );
}

void CommandBuffer::setViewport( const cyd::Rectangle& viewport ) const
{
   VkViewport vkViewport = {
       viewport.offset.x,
       viewport.offset.y,
       static_cast<float>( viewport.extent.width ),
       static_cast<float>( viewport.extent.height ),
       0.0f,
       1.0f };
   vkCmdSetViewport( m_vkCmdBuffer, 0, 1, &vkViewport );
}

void CommandBuffer::beginPass( const cyd::RenderPassInfo& renderPassInfo, Swapchain& swapchain )
{
   VkRenderPass renderPass = m_pDevice->getRenderPassStash().findOrCreate( renderPassInfo );
   CYDASSERT( renderPass && "CommandBuffer: Could not find render pass" );

   m_boundRenderPass = renderPass;

   swapchain.initFramebuffers( renderPassInfo, renderPass );
   swapchain.acquireImage( this );

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

void CommandBuffer::_prepareDescriptorSets()
{
   // Allocating descriptor sets
   const size_t prevSize = m_descSets.size();

   m_descSets.reserve( prevSize + m_boundPipInfo.value().pipLayout.descSets.size() );
   for( const auto& descSet : m_boundPipInfo.value().pipLayout.descSets )
   {
      m_descSets.push_back( m_pDevice->getDescriptorPool().allocate( descSet ) );
   }

   // Creating write descriptors for resources we want to update for this draw
   const size_t totalSize = m_buffersToUpdate.size() + m_texturesToUpdate.size();
   std::vector<VkDescriptorBufferInfo> bufferInfos;
   std::vector<VkDescriptorImageInfo> imageInfos;
   std::vector<VkWriteDescriptorSet> writeDescSets;
   writeDescSets.reserve( totalSize );
   bufferInfos.reserve( totalSize );
   imageInfos.reserve( totalSize );

   for( const auto& entry : m_buffersToUpdate )
   {
      VkDescriptorBufferInfo bufferInfo;
      bufferInfo.buffer = entry.buffer->getVKBuffer();
      bufferInfo.offset = 0;
      bufferInfo.range  = entry.buffer->getSize();
      bufferInfos.push_back( bufferInfo );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet               = m_descSets[prevSize + entry.set];
      descriptorWrite.dstBinding           = entry.binding;
      descriptorWrite.dstArrayElement      = 0;
      descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount      = 1;
      descriptorWrite.pBufferInfo          = &bufferInfos.back();

      writeDescSets.push_back( descriptorWrite );
   }

   for( const auto& entry : m_texturesToUpdate )
   {
      const VkSampler vkSampler = m_pDevice->getSamplerStash().findOrCreate( {} );

      VkDescriptorImageInfo imageInfo = {};
      // By the point we bind this texture, it should have transferred to this layout
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
      imageInfo.imageView   = entry.texture->getVKImageView();
      imageInfo.sampler     = vkSampler;
      imageInfos.push_back( imageInfo );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet               = m_descSets[prevSize + entry.set];
      descriptorWrite.dstBinding           = entry.binding;
      descriptorWrite.dstArrayElement      = 0;
      descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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
   vkCmdBindDescriptorSets(
       m_vkCmdBuffer,
       VK_PIPELINE_BIND_POINT_GRAPHICS,
       m_boundPipLayout.value(),
       0,
       static_cast<uint32_t>( m_descSets.size() - prevSize ),
       &m_descSets[prevSize],
       0,
       nullptr );

   m_buffersToUpdate.clear();
   m_texturesToUpdate.clear();
}

void CommandBuffer::draw( size_t vertexCount )
{
   CYDASSERT(
       m_usage & cyd::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command buffer does not support graphics usage" );

   CYDASSERT(
       m_boundPipInfo.has_value() && "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets();

   vkCmdDraw( m_vkCmdBuffer, static_cast<uint32_t>( vertexCount ), 1, 0, 0 );
}

void CommandBuffer::drawIndexed( size_t indexCount )
{
   CYDASSERT(
       m_usage & cyd::QueueUsage::GRAPHICS &&
       "CommandBuffer: Command Buffer does not support graphics usage" );

   CYDASSERT(
       m_boundPipInfo.has_value() && "CommandBuffer: Cannot draw because no pipeline was bound" );

   _prepareDescriptorSets();

   vkCmdDrawIndexed( m_vkCmdBuffer, static_cast<uint32_t>( indexCount ), 1, 0, 0, 0 );
}

void CommandBuffer::endPass() const { vkCmdEndRenderPass( m_vkCmdBuffer ); }

void CommandBuffer::copyBuffer( const Buffer* src, const Buffer* dst ) const
{
   CYDASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   VkBufferCopy copyRegion = {};
   copyRegion.size         = dst->getSize();
   vkCmdCopyBuffer( m_vkCmdBuffer, src->getVKBuffer(), dst->getVKBuffer(), 1, &copyRegion );
}

void CommandBuffer::uploadBufferToTex( const Buffer* src, Texture* dst )
{
   CYDASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   // Transition image layout to transfer destination optimal
   VkImageMemoryBarrier barrier = {};
   barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout            = TypeConversions::cydImageLayoutToVKImageLayout( m_prevLayout );
   barrier.newLayout            = VK_IMAGE_LAYOUT_GENERAL;
   barrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
   barrier.image                = dst->getVKImage();
   barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.baseMipLevel   = 0;
   barrier.subresourceRange.levelCount     = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount     = 1;
   barrier.srcAccessMask                   = 0;
   barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;

   vkCmdPipelineBarrier(
       m_vkCmdBuffer,
       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
       VK_PIPELINE_STAGE_TRANSFER_BIT,
       0,
       0,
       nullptr,
       0,
       nullptr,
       1,
       &barrier );

   // Updating current image layout
   m_prevLayout = cyd::ImageLayout::GENERAL;

   // Copying data from buffer to texture
   VkBufferImageCopy region               = {};
   region.bufferOffset                    = 0;
   region.bufferRowLength                 = 0;
   region.bufferImageHeight               = 0;
   region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   region.imageSubresource.mipLevel       = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount     = 1;
   region.imageOffset                     = { 0, 0, 0 };
   region.imageExtent                     = { dst->getWidth(), dst->getHeight(), 1 };

   vkCmdCopyBufferToImage(
       m_vkCmdBuffer, src->getVKBuffer(), dst->getVKImage(), VK_IMAGE_LAYOUT_GENERAL, 1, &region );
}

void CommandBuffer::submit()
{
   VkSubmitInfo submitInfo       = {};
   submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers    = &m_vkCmdBuffer;

   VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
   submitInfo.waitSemaphoreCount     = static_cast<uint32_t>( m_semsToWait.size() );
   submitInfo.pWaitSemaphores        = m_semsToWait.data();
   submitInfo.pWaitDstStageMask      = waitStages;
   submitInfo.signalSemaphoreCount   = static_cast<uint32_t>( m_semsToSignal.size() );
   submitInfo.pSignalSemaphores      = m_semsToSignal.data();

   const VkQueue* queue = m_pDevice->getQueueFromFamily( m_pPool->getFamilyIndex() );
   CYDASSERT( queue && "CommandBuffer: Could not find queue to submit to" );

   if( m_wasSubmitted )
   {
      // CPU Lock
      vkWaitForFences( m_pDevice->getVKDevice(), 1, &m_vkFence, true, 0 );
      vkResetFences( m_pDevice->getVKDevice(), 1, &m_vkFence );
   }

   vkQueueSubmit( *queue, 1, &submitInfo, m_vkFence );
   m_wasSubmitted = true;

   m_semsToWait.clear();
   m_semsToSignal.clear();
}
}  // namespace vk
