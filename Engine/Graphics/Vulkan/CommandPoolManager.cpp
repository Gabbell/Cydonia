#include <Graphics/Vulkan/CommandPoolManager.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/CommandBufferPool.h>

namespace vk
{
CommandPoolManager::CommandPoolManager( const Device& device, const uint32_t nbFamilies )
    : m_device( device ), m_nbFamilies( nbFamilies )
{
   // This should be the main thread/initialization thread
   const std::thread::id threadId = std::this_thread::get_id();

   _initializePoolsForThread( threadId );
}

CommandBuffer* CommandPoolManager::acquire(
    CYD::QueueUsageFlag usage,
    const std::string_view name,
    bool presentable )
{
   // TODO Implement support for EXACT type so that we can possibly send work (like transfer for
   // example) to another queue family
   const std::thread::id threadId = std::this_thread::get_id();

   const auto it = m_commandPools.find( threadId );
   if( it == m_commandPools.end() )
   {
      _initializePoolsForThread( threadId );
   }

   // Attempting to find an adequate type
   const auto poolIt = std::find_if(
       it->second.begin(), it->second.end(), [usage, presentable]( const CommandBufferPool& pool ) {
          return ( usage & pool.getType() ) && !( presentable && !pool.supportsPresentation() );
       } );

   if( poolIt != it->second.end() )
   {
      // Found an adequate command pool
      return poolIt->createCommandBuffer( usage, name );
   }

   return nullptr;
}

void CommandPoolManager::cleanup()
{
   for( auto& poolsPerThread : m_commandPools )
   {
      for( auto& pool : poolsPerThread.second )
      {
         pool.cleanup();
      }
   }
}

void CommandPoolManager::_initializePoolsForThread( const std::thread::id threadId )
{
   for( uint32_t familyIdx = 0; familyIdx < m_nbFamilies; ++familyIdx )
   {
      const Device::QueueFamily& family = m_device.getQueueFamilyFromIndex( familyIdx );

      m_commandPools[threadId].emplace_back(
          m_device, familyIdx, family.type, family.supportsPresent );
   }
}
}