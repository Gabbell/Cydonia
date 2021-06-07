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
   CommandPoolManager( const Device& device, const uint32_t nbFamilies );
   NON_COPIABLE( CommandPoolManager );
   ~CommandPoolManager() = default;

   CommandBuffer*
   acquire( CYD::QueueUsageFlag usage, const std::string_view name, bool presentable );

   void cleanup();

  private:
   void _initializePoolsForThread( const std::thread::id threadId );

   const Device& m_device;

   uint32_t m_nbFamilies;

   using PoolPerQueueFamilyIndex = std::vector<CommandBufferPool>;

   std::unordered_map<std::thread::id, PoolPerQueueFamilyIndex> m_commandPools;
};
}
