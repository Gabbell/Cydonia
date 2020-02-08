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
    : m_pDevice( &device ),
      m_type( type ),
      m_familyIndex( familyIndex ),
      m_supportsPresentation( supportsPresentation )
{
   // This is necessary to prevent resizing and invalidating all pointers in the handle manager
   m_cmdBuffers.resize( MAX_CMD_BUFFERS_IN_FLIGHT );

   _createCommandPool();
}

void CommandPool::_createCommandPool()
{
   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex        = m_familyIndex;
   poolInfo.flags =
       VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

   VkResult result = vkCreateCommandPool( m_pDevice->getVKDevice(), &poolInfo, nullptr, &m_vkPool );
   CYDASSERT( result == VK_SUCCESS && "CommandPool: Could not create command pool" );
}

CommandBuffer* CommandPool::createCommandBuffer()
{
   // Check to see if we have a free spot for a command buffer. Either one that has never been
   // allocated (no VK command buffer handle) or one that is completed.
   auto it =
       std::find_if( m_cmdBuffers.rbegin(), m_cmdBuffers.rend(), []( CommandBuffer& cmdBuffer ) {
          return !cmdBuffer.getVKBuffer() || cmdBuffer.isCompleted();
       } );

   if( it != m_cmdBuffers.rend() )
   {
      // We found a completed command buffer that can be replaced
      it->release();
      it->acquire( *m_pDevice, *this, m_type );
      return &*it;
   }

   CYDASSERT( !"CommandPool: Too many command buffers in flight" );
   return nullptr;
}

CommandPool::~CommandPool()
{
   for( auto& cmdBuffer : m_cmdBuffers )
   {
      cmdBuffer.release();
   }
   vkDestroyCommandPool( m_pDevice->getVKDevice(), m_vkPool, nullptr );
}
}
