#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <cstdint>
#include <thread>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
namespace vk
{
class Device;
class CommandBufferPool;
class CommandBuffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class CommandPoolManager final
{
  public:
   CommandPoolManager() = delete;
   CommandPoolManager( const Device& device, const uint32_t nbFamilies, const uint32_t nbFrames );
   NON_COPIABLE( CommandPoolManager );
   ~CommandPoolManager() = default;

   CommandBuffer* acquire(
       CYD::QueueUsageFlag usage,
       const std::string_view name,
       bool presentable,
       uint32_t currentFrame );

   void waitOnFrame( uint32_t currentFrame );

   void cleanup();

  private:
   void _initializePoolsForThread( const std::thread::id threadId );

   const Device& m_device;

   uint32_t m_nbFamilies;
   uint32_t m_nbFrames;

   using PoolsPerQueueFamily         = std::vector<CommandBufferPool>;  // Number of queue families
   using PoolsPerQueueFamilyPerFrame = std::vector<PoolsPerQueueFamily>;  // In-flight frames

   std::unordered_map<std::thread::id, PoolsPerQueueFamilyPerFrame> m_commandPools;
};
}
