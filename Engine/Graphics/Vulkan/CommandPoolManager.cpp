#include <Graphics/Vulkan/CommandPoolManager.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/CommandBufferPool.h>

#include <Common/Assert.h>

namespace vk
{
CommandPoolManager::CommandPoolManager(
    const Device& device,
    const uint32_t nbFamilies,
    const uint32_t nbFrames )
    : m_device( device ), m_nbFamilies( nbFamilies ), m_nbFrames( nbFrames )
{
   // This should be the main thread/initialization thread
   const std::thread::id threadId = std::this_thread::get_id();

   _initializePoolsForThread( threadId );
}

CommandBuffer* CommandPoolManager::acquire(
    CYD::QueueUsageFlag usage,
    const std::string_view name,
    bool presentable,
    const uint32_t currentFrame )
{
   // TODO Implement support for EXACT type so that we can possibly send work (like transfer for
   // example) to another queue family
   const std::thread::id threadId = std::this_thread::get_id();

   const auto it = m_commandPools.find( threadId );
   if( it == m_commandPools.end() )
   {
      // There were no command pools found for this thread, create them
      _initializePoolsForThread( threadId );
   }

   // Attempting to find a pool of the adequate type
   PoolsPerQueueFamily& poolsPerQueueFamily = it->second[currentFrame];

   const auto poolIt = std::find_if(
       poolsPerQueueFamily.begin(),
       poolsPerQueueFamily.end(),
       [usage, presentable]( const CommandBufferPool& pool )
       { return ( usage & pool.getType() ) && !( presentable && !pool.supportsPresentation() ); } );

   if( poolIt != poolsPerQueueFamily.end() )
   {
      // Found an adequate command pool
      return poolIt->createCommandBuffer( usage, name );
   }

   CYD_ASSERT( !"Failed to find adequate command pool" );
   return nullptr;
}

void CommandPoolManager::waitOnFrame( uint32_t currentFrame )
{
   const std::thread::id threadId = std::this_thread::get_id();

   const auto it = m_commandPools.find( threadId );

   PoolsPerQueueFamily& poolsPerFamily = it->second[currentFrame];
   for( auto& poolPerFamily : poolsPerFamily )
   {
      poolPerFamily.waitUntilDone();
   }
}

void CommandPoolManager::cleanup()
{
   for( auto& poolsPerThread : m_commandPools )
   {
      for( auto& poolPerFrame : poolsPerThread.second )
      {
         for( auto& poolPerFamily : poolPerFrame )
         {
            poolPerFamily.cleanup();
         }
      }
   }
}

void CommandPoolManager::_initializePoolsForThread( const std::thread::id threadId )
{
   m_commandPools[threadId].resize( m_nbFrames );

   for( uint32_t imageIdx = 0; imageIdx < m_nbFrames; ++imageIdx )
   {
      m_commandPools[threadId][imageIdx].reserve( m_nbFamilies );

      for( uint32_t familyIdx = 0; familyIdx < m_nbFamilies; ++familyIdx )
      {
         const Device::QueueFamily& family = m_device.getQueueFamilyFromIndex( familyIdx );

         m_commandPools[threadId][imageIdx].emplace_back(
             m_device, familyIdx, family.type, family.supportsPresent );
      }
   }
}
}