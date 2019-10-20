#include <Core/Graphics/CommandBuffer.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/CommandPool.h>
#include <Core/Graphics/Device.h>
#include <Core/Graphics/PipelineStash.h>
#include <Core/Graphics/RenderPassStash.h>
#include <Core/Graphics/Swapchain.h>

cyd::CommandBuffer::CommandBuffer( const Device& device, const CommandPool& pool, UsageFlag usage )
    : _device( device ), _pool( pool ), _usage( usage )
{
   VkCommandBufferAllocateInfo allocInfo = {};
   allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool                 = _pool.getVKCommandPool();
   allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount          = 1;

   VkResult result = vkAllocateCommandBuffers( _device.getVKDevice(), &allocInfo, &_vkBuffer );
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

void cyd::CommandBuffer::startRecording()
{
   if( !_isRecording )
   {
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      VkResult result = vkBeginCommandBuffer( _vkBuffer, &beginInfo );
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
      VkResult result = vkEndCommandBuffer( _vkBuffer );
      CYDASSERT(
          result == VK_SUCCESS && "CommandBuffer: Failed to end recording of command buffer" );

      _boundPipeline = std::nullopt;
      _isRecording   = false;
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Trying to stop recording but was not in recording state" );
   }
}

void cyd::CommandBuffer::setPipeline( const PipelineInfo& info )
{
   VkPipeline pipeline = _device.getPipelineStash().findOrCreate( info );
   if( pipeline )
   {
      vkCmdBindPipeline( _vkBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
      _boundPipeline = info;
   }
   else
   {
      CYDASSERT( !"CommandBuffer: Could not find pipeline in pipeline stash" );
   }
}

void cyd::CommandBuffer::setViewport( uint32_t width, uint32_t height )
{
   VkViewport viewport = {
       0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 1.0f};
   vkCmdSetViewport( _vkBuffer, 0, 1, &viewport );
}

void cyd::CommandBuffer::beginPass( Swapchain* swapchain )
{
   if( _boundPipeline.has_value() )
   {
      VkRenderPass renderPass =
          _device.getRenderPassStash().findOrCreate( _boundPipeline.value().renderPass );

      if( renderPass && swapchain )
      {
         swapchain->initFramebuffers( renderPass );
         swapchain->acquireImage( this );

         VkRenderPassBeginInfo renderPassInfo = {};
         renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
         renderPassInfo.renderPass            = renderPass;
         renderPassInfo.framebuffer           = swapchain->getCurrentFramebuffer();
         renderPassInfo.renderArea.offset     = {0, 0};
         renderPassInfo.renderArea.extent     = swapchain->getVKExtent();

         VkClearValue clearColor        = {1.0f, 0.0f, 0.0f, 1.0f};
         renderPassInfo.clearValueCount = 1;
         renderPassInfo.pClearValues    = &clearColor;

         vkCmdBeginRenderPass( _vkBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
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

void cyd::CommandBuffer::draw() { vkCmdDraw( _vkBuffer, 3, 1, 0, 0 ); }

void cyd::CommandBuffer::endPass() { vkCmdEndRenderPass( _vkBuffer ); }

void cyd::CommandBuffer::submit() {}

cyd::CommandBuffer::~CommandBuffer()
{
   vkDestroyFence( _device.getVKDevice(), _vkFence, nullptr );
   vkFreeCommandBuffers( _device.getVKDevice(), _pool.getVKCommandPool(), 1, &_vkBuffer );
};
