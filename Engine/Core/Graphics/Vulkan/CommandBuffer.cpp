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
   if( !_isRecording )
   {
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      VkResult result = vkBeginCommandBuffer( _vkCmdBuffer, &beginInfo );
      CYDASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to begin recording of command buffer" );
      _isRecording = true;
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Already started recording" );
   }
}

void cyd::CommandBuffer::endRecording()
{
   if( _isRecording )
   {
      VkResult result = vkEndCommandBuffer( _vkCmdBuffer );
      CYDASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to end recording of command buffer" );

      _boundPip       = std::nullopt;
      _boundPipLayout = std::nullopt;
      _isRecording    = false;
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Trying to stop recording but was not in recording state" );
   }
}

void cyd::CommandBuffer::pushConstants( const PipelineLayoutInfo& info, ShaderStage stage ) {}

void cyd::CommandBuffer::bindPipeline( const PipelineInfo& info )
{
   VkPipeline pipeline = _device.getPipelineStash().findOrCreate( info );
   if( pipeline )
   {
      vkCmdBindPipeline( _vkCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
      _boundPip = info;
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Could not find or create pipeline in pipeline stash" );
   }
}

void cyd::CommandBuffer::bindVertexBuffer( const std::shared_ptr<Buffer> vertexBuffer )
{
   VkBuffer vertexBuffers[] = { vertexBuffer->getVKBuffer() };
   VkDeviceSize offsets[]   = { 0 };
   vkCmdBindVertexBuffers( _vkCmdBuffer, 0, 1, vertexBuffers, offsets );
}

void cyd::CommandBuffer::setViewport( uint32_t width, uint32_t height )
{
   VkViewport viewport = {
       0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 1.0f };
   vkCmdSetViewport( _vkCmdBuffer, 0, 1, &viewport );
}

void cyd::CommandBuffer::beginPass( Swapchain* swapchain )
{
   if( _boundPip.has_value() )
   {
      VkRenderPass renderPass =
          _device.getRenderPassStash().findOrCreate( _boundPip.value().renderPass );

      if( renderPass && swapchain )
      {
         swapchain->initFramebuffers( renderPass );
         swapchain->acquireImage( this );

         VkRenderPassBeginInfo renderPassInfo = {};
         renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
         renderPassInfo.renderPass            = renderPass;
         renderPassInfo.framebuffer           = swapchain->getCurrentFramebuffer();
         renderPassInfo.renderArea.offset     = { 0, 0 };
         renderPassInfo.renderArea.extent     = swapchain->getVKExtent();

         VkClearValue clearColor        = { 0.0f, 1.0f, 1.0f, 1.0f };
         renderPassInfo.clearValueCount = 1;
         renderPassInfo.pClearValues    = &clearColor;

         vkCmdBeginRenderPass( _vkCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
      }
      else
      {
         CYDASSERT( !"CommandBuffer: Could not find render pass in render pass stash" );
      }
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Could not start render pass because no pipeline was bound" );
   }
}

void cyd::CommandBuffer::draw()
{
   if( _usage & QueueUsage::GRAPHICS )
   {
      vkCmdDraw( _vkCmdBuffer, 3, 1, 0, 0 );
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

   // TODO QUEUE MUST COME FROM THE QUEUE FAMLIY FROM WHICH THE COMMAND POOL WAS CREATED
   const VkQueue* queue = _device.getQueue( _pool.getFamilyIndex(), false );
   if( queue )
   {
      vkQueueSubmit( *queue, 1, &submitInfo, _vkFence );
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Could not find queue to submit to" );
   }
}

cyd::CommandBuffer::~CommandBuffer()
{
   vkDestroyFence( _device.getVKDevice(), _vkFence, nullptr );
   vkFreeCommandBuffers( _device.getVKDevice(), _pool.getVKCommandPool(), 1, &_vkCmdBuffer );
};
