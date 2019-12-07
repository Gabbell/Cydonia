#include <Graphics/Vulkan/CommandPool.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/CommandBuffer.h>

#include <algorithm>

namespace vk
{
CommandPool::CommandPool(
    const Device& device,
    uint32_t familyIndex,
    cyd::QueueUsageFlag type,
    bool supportsPresentation )
    : _device( device ),
      _type( type ),
      _familyIndex( familyIndex ),
      _supportsPresentation( supportsPresentation )
{
   // This is necessary to prevent resizing and invalidating all pointers in the handle manager
   _cmdBuffers.resize( MAX_CMD_BUFFERS_IN_FLIGHT );

   _createCommandPool();
}

void CommandPool::_createCommandPool()
{
   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex        = _familyIndex;
   poolInfo.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

   VkResult result = vkCreateCommandPool( _device.getVKDevice(), &poolInfo, nullptr, &_vkPool );
   CYDASSERT( result == VK_SUCCESS && "CommandPool: Could not create command pool" );
}

CommandBuffer* CommandPool::createCommandBuffer()
{
   // Check to see if we have a free spot for a command buffer. Either one that has never been
   // allocated (no VK command buffer handle) or one that is completed.
   auto it =
       std::find_if( _cmdBuffers.rbegin(), _cmdBuffers.rend(), []( CommandBuffer& cmdBuffer ) {
          return !cmdBuffer.getVKBuffer() || cmdBuffer.isCompleted();
       } );

   if( it != _cmdBuffers.rend() )
   {
      // We found a completed command buffer that can be replaced
      it->release();
      it->seize( _device, *this, _type );
      return &*it;
   }

   CYDASSERT( !"CommandPool: Too many command buffers in flight" );
   return nullptr;
}

CommandPool::~CommandPool()
{
   for( auto& cmdBuffer : _cmdBuffers )
   {
      cmdBuffer.release();
   }
   vkDestroyCommandPool( _device.getVKDevice(), _vkPool, nullptr );
}
}  // namespace vk
