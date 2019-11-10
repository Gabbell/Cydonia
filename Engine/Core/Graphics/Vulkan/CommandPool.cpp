#include <Core/Graphics/Vulkan/CommandPool.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/CommandBuffer.h>

static constexpr uint32_t INITIAL_SIZE = 64;
static constexpr uint32_t MAX_SIZE     = 1024;

cyd::CommandPool::CommandPool( const Device& device, uint32_t familyIndex, QueueUsageFlag type )
    : _device( device ), _familyIndex( familyIndex ), _type( type )
{
   _createCommandPool();
}

void cyd::CommandPool::_createCommandPool()
{
   _buffers.resize( INITIAL_SIZE );

   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex        = _familyIndex;
   poolInfo.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

   VkResult result = vkCreateCommandPool( _device.getVKDevice(), &poolInfo, nullptr, &_vkPool );
   CYDASSERT( result == VK_SUCCESS && "CommandPool: Could not create command pool" );
}

std::shared_ptr<cyd::CommandBuffer> cyd::CommandPool::createCommandBuffer()
{
   while( _buffers.size() < MAX_SIZE )
   {
      auto it = std::find( _buffers.begin(), _buffers.end(), nullptr );

      if( it != _buffers.end() )
      {
         // There is a free spot
         *it = std::make_shared<CommandBuffer>( _device, *this, _type );
         return *it;
      }
      else
      {
         _buffers.resize( _buffers.size() * 2 );
      }
   }

   CYDASSERT( !"CommandPool: Ran out of slots for command buffers" );
   return nullptr;
}

void cyd::CommandPool::cleanup()
{
   for( auto& cmdBuffer : _buffers )
   {
      if( cmdBuffer.use_count() == 1 && cmdBuffer->isCompleted() )
      {
         cmdBuffer.reset();
      }
   }
}

cyd::CommandPool::~CommandPool()
{
   for( auto& cmdBuffer : _buffers )
   {
      cmdBuffer.reset();
   }

   vkDestroyCommandPool( _device.getVKDevice(), _vkPool, nullptr );
}
