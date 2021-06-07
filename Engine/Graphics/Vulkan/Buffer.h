#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <atomic>
#include <cstdint>
#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkBuffer );
FWDHANDLE( VkDeviceMemory );
FWDHANDLE( VkDescriptorSet );

namespace vk
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Buffer final
{
  public:
   Buffer();
   MOVABLE( Buffer );
   ~Buffer();

   void acquire(
       const Device& device,
       size_t size,
       CYD::BufferUsageFlag usage,
       CYD::MemoryTypeFlag memoryType,
       const std::string_view name );
   void release();

   size_t getSize() const noexcept { return m_size; }
   VkBuffer getVKBuffer() const noexcept { return m_vkBuffer; }
   bool inUse() const { return ( *m_useCount ) > 0; }

   void incUse();
   void decUse();

   void copy( const void* pData, size_t offset, size_t size );

  private:
   void _mapMemory();
   void _unmapMemory();
   void _allocateMemory();

   const Device* m_pDevice = nullptr;

   // Used for staging buffers
   void* m_data = nullptr;

   // Common
   static constexpr char DEFAULT_BUFFER_NAME[] = "Unknown Buffer Name";

   std::string_view m_name   = DEFAULT_BUFFER_NAME;
   size_t m_size             = 0;
   VkBuffer m_vkBuffer       = nullptr;
   VkDeviceMemory m_vkMemory = nullptr;
   CYD::MemoryTypeFlag m_memoryType;

   std::unique_ptr<std::atomic<uint32_t>> m_useCount;
};
}
