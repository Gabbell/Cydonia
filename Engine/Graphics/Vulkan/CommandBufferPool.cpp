#include <Graphics/Vulkan/CommandBufferPool.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/CommandBuffer.h>

#include <algorithm>

namespace vk
{
CommandBufferPool::CommandBufferPool(
    const Device& device,
    uint32_t familyIndex,
    CYD::QueueUsageFlag type,
    bool supportsPresentation )
    : m_pDevice( &device ),
      m_type( type ),
      m_familyIndex( familyIndex ),
      m_supportsPresentation( supportsPresentation )
{
   // This is necessary to prevent resizing and invalidating all pointers in the handle manager
   m_cmdBuffers.resize( MAX_CMD_BUFFERS_IN_FLIGHT );

   _createCommandBufferPool();
}

CommandBuffer* CommandBufferPool::createCommandBuffer(
    CYD::QueueUsageFlag usage,
    const std::string_view name )
{
   // Check to see if we have a free spot for a command buffer. Either one that is already
   // released or one that has been completed, freed and is not in use anymore
   auto it = std::find_if(
       m_cmdBuffers.rbegin(),
       m_cmdBuffers.rend(),
       []( CommandBuffer& cmdBuffer )
       { return cmdBuffer.isReleased() || ( cmdBuffer.isFree() && !cmdBuffer.inUse() ); } );

   if( it != m_cmdBuffers.rend() )
   {
      // We found an unused command buffer that can be replaced. Make sure its previous sync
      // objects are released
      if( it->isFree() )
      {
         it->release();
      }

      it->acquire( *m_pDevice, *this, usage, name );
      m_cmdBuffersInUse++;
      return &*it;
   }

   CYD_ASSERT( !"CommandBufferPool: Too many command buffers in flight" );
   return nullptr;
}

void CommandBufferPool::waitUntilDone()
{
   // Too many command buffers in flight, CPU wait until one of the fences gets signaled
   std::vector<VkFence> vkFences;
   vkFences.reserve( m_cmdBuffers.size() );
   for( CommandBuffer& cmdBuffer : m_cmdBuffers )
   {
      if( cmdBuffer.isSubmitted() )
      {
         vkFences.push_back( cmdBuffer.getVKFence() );
      }
   }

   if( !vkFences.empty() )
   {
      const VkResult result = vkWaitForFences(
          m_pDevice->getVKDevice(),
          static_cast<uint32_t>( vkFences.size() ),
          vkFences.data(),
          false,
          UINT64_MAX );
   }
}

void CommandBufferPool::cleanup()
{
   for( auto& cmdBuffer : m_cmdBuffers )
   {
      if( cmdBuffer.isSubmitted() && cmdBuffer.isCompleted() )
      {
         // This command buffer is valid and has finished execution. Release the resources linked to
         // it but not the sync objects as other parts of the API could still be using them
         cmdBuffer.free();
         m_cmdBuffersInUse--;
      }
   }
}

void CommandBufferPool::_createCommandBufferPool()
{
   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex        = m_familyIndex;
   poolInfo.flags =
       VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

   VkResult result = vkCreateCommandPool( m_pDevice->getVKDevice(), &poolInfo, nullptr, &m_vkPool );
   CYD_ASSERT( result == VK_SUCCESS && "CommandBufferPool: Could not create command pool" );
}

CommandBufferPool::~CommandBufferPool()
{
   cleanup();
   vkDestroyCommandPool( m_pDevice->getVKDevice(), m_vkPool, nullptr );
}
}
