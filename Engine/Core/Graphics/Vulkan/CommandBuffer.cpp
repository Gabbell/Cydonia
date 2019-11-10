#include <Core/Graphics/Vulkan/CommandBuffer.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/CommandPool.h>
#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/PipelineStash.h>
#include <Core/Graphics/Vulkan/RenderPassStash.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/Buffer.h>

cyd::CommandBuffer::CommandBuffer(
    const Device& device,
    const CommandPool& pool,
    QueueUsageFlag usage )
    : _device( device ), _pool( pool ), _usage( usage )
{
   VkCommandBufferAllocateInfo allocInfo = {};
   allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool                 = _pool.getVKCommandPool();
   allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount          = 1;

   VkResult result = vkAllocateCommandBuffers( _device.getVKDevice(), &allocInfo, &_vkCmdBuffer );
   CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Could not allocate command buffer" );

   VkFenceCreateInfo fenceInfo = {};
   fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

   result = vkCreateFence( _device.getVKDevice(), &fenceInfo, nullptr, &_vkFence );
   CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Could not create fence" );
}

bool cyd::CommandBuffer::isCompleted() const
{
   return vkGetFenceStatus( _device.getVKDevice(), _vkFence ) == VK_SUCCESS;
}

void cyd::CommandBuffer::waitForCompletion() const
{
   while( vkGetFenceStatus( _device.getVKDevice(), _vkFence ) != VK_SUCCESS )
      ;
}

void cyd::CommandBuffer::startRecording()
{
   if( _isRecording )
   {
      CYDASSERT( !"CommandBuffer: Already started recording" );
      return;
   }

   VkCommandBufferBeginInfo beginInfo = {};
   beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

   VkResult result = vkBeginCommandBuffer( _vkCmdBuffer, &beginInfo );
   CYDASSERT(
       result == VK_SUCCESS && "CommandBuffer: Failed to begin recording of command buffer" );
   _isRecording = true;
}

void cyd::CommandBuffer::endRecording()
{
   if( !_isRecording )
   {
      CYDASSERT( !"CommandBuffer: Trying to stop recording but was not in recording state" );
      return;
   }

   VkResult result = vkEndCommandBuffer( _vkCmdBuffer );
   CYDASSERT( result == VK_SUCCESS && "CommandBuffer: Failed to end recording of command buffer" );

   _boundPip       = std::nullopt;
   _boundPipLayout = std::nullopt;
   _isRecording    = false;
}

void cyd::CommandBuffer::updatePushConstants( PushConstantRange range, void* data )
{
   if( !_boundPipLayout.has_value() )
   {
      CYDASSERT( !"CommandBuffer: No currently bound pipeline layout" );
      return;
   }

   vkCmdPushConstants(
       _vkCmdBuffer,
       _boundPipLayout.value(),
       cydShaderStagesToVkShaderStages( range.stages ),
       range.offset,
       range.size,
       data );
}

void cyd::CommandBuffer::bindPipeline( const PipelineInfo& info )
{
   VkPipeline pipeline        = _device.getPipelineStash().findOrCreate( info );
   VkPipelineLayout pipLayout = _device.getPipelineStash().findOrCreate( info.pipLayout );
   VkRenderPass renderPass    = _device.getRenderPassStash().findOrCreate( info.renderPass );

   if( !pipeline || !pipLayout || !renderPass )
   {
      CYDASSERT( !"CommandBuffer: Could not find or create pipeline in pipeline stash" );
      return;
   }

   vkCmdBindPipeline( _vkCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
   _boundPip        = pipeline;
   _boundPipLayout  = pipLayout;
   _boundRenderPass = renderPass;
}

void cyd::CommandBuffer::bindVertexBuffer( const std::shared_ptr<Buffer> vertexBuffer )
{
   VkBuffer vertexBuffers[] = { vertexBuffer->getVKBuffer() };
   VkDeviceSize offsets[]   = { 0 };
   vkCmdBindVertexBuffers( _vkCmdBuffer, 0, 1, vertexBuffers, offsets );
}

void cyd::CommandBuffer::bindBuffer( const std::shared_ptr<Buffer> buffer )
{
   if( !_boundPipLayout.has_value() )
   {
      CYDASSERT( !"CommandBuffer: Not currently bound pipeline" );
      return;
   }

   vkCmdBindDescriptorSets(
       _vkCmdBuffer,
       VK_PIPELINE_BIND_POINT_GRAPHICS,
       _boundPipLayout.value(),
       0,
       1,
       &buffer->getVKDescSet(),
       0,
       nullptr );
}

void cyd::CommandBuffer::setViewport( uint32_t width, uint32_t height )
{
   VkViewport viewport = {
       0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 1.0f };
   vkCmdSetViewport( _vkCmdBuffer, 0, 1, &viewport );
}

void cyd::CommandBuffer::beginPass( Swapchain* swapchain )
{
   if( !_boundPip.has_value() || !_boundRenderPass.has_value() )
   {
      CYDASSERT( !"CommandBuffer: Could not start render pass because no pipeline was bound" );
      return;
   }

   if( !swapchain )
   {
      CYDASSERT( !"CommandBuffer: Could not find render pass or swapchain was null" );
      return;
   }

   swapchain->initFramebuffers( _boundRenderPass.value() );
   swapchain->acquireImage( this );

   _semsToWait.push_back( swapchain->getSemToWait() );
   _semsToSignal.push_back( swapchain->getSemToSignal() );

   VkRenderPassBeginInfo renderPassInfo = {};
   renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassInfo.renderPass            = _boundRenderPass.value();
   renderPassInfo.framebuffer           = swapchain->getCurrentFramebuffer();
   renderPassInfo.renderArea.offset     = { 0, 0 };
   renderPassInfo.renderArea.extent     = swapchain->getVKExtent();

   VkClearValue clearColor        = { 0.0f, 1.0f, 1.0f, 1.0f };
   renderPassInfo.clearValueCount = 1;
   renderPassInfo.pClearValues    = &clearColor;

   vkCmdBeginRenderPass( _vkCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void cyd::CommandBuffer::draw( uint32_t vertexCount )
{
   if( _usage & QueueUsage::GRAPHICS )
   {
      vkCmdDraw( _vkCmdBuffer, vertexCount, 1, 0, 0 );
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Command Buffer does not support graphics usage" );
   }
}

void cyd::CommandBuffer::endPass() { vkCmdEndRenderPass( _vkCmdBuffer ); }

void cyd::CommandBuffer::copyBuffer(
    const std::shared_ptr<Buffer> src,
    const std::shared_ptr<Buffer> dst )
{
   CYDASSERT(
       src->getSize() == dst->getSize() &&
       "CommandBuffer: Source and destination sizes are not the same" );

   VkBufferCopy copyRegion = {};
   copyRegion.size         = dst->getSize();
   vkCmdCopyBuffer( _vkCmdBuffer, src->getVKBuffer(), dst->getVKBuffer(), 1, &copyRegion );
}

void cyd::CommandBuffer::submit()
{
   VkSubmitInfo submitInfo       = {};
   submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers    = &_vkCmdBuffer;

   VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
   submitInfo.waitSemaphoreCount     = static_cast<uint32_t>( _semsToWait.size() );
   submitInfo.pWaitSemaphores        = _semsToWait.data();
   submitInfo.pWaitDstStageMask      = waitStages;
   submitInfo.signalSemaphoreCount   = static_cast<uint32_t>( _semsToSignal.size() );
   submitInfo.pSignalSemaphores      = _semsToSignal.data();

   const VkQueue* queue = _device.getQueueFromFamily( _pool.getFamilyIndex() );
   if( !queue )
   {
      CYDASSERT( !"CommandBuffer: Could not find queue to submit to" );
      return;
   }

   vkQueueSubmit( *queue, 1, &submitInfo, _vkFence );
}

cyd::CommandBuffer::~CommandBuffer()
{
   vkDestroyFence( _device.getVKDevice(), _vkFence, nullptr );
   vkFreeCommandBuffers( _device.getVKDevice(), _pool.getVKCommandPool(), 1, &_vkCmdBuffer );
};
