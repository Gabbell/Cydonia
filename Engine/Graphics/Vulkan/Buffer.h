#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <cstdint>

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
   Buffer() = default;
   MOVABLE( Buffer )
   ~Buffer();

   void acquire(
       const Device& device,
       size_t size,
       CYD::BufferUsageFlag usage,
       CYD::MemoryTypeFlag memoryType );
   void release();

   size_t getSize() const noexcept { return m_size; }
   VkBuffer getVKBuffer() const noexcept { return m_vkBuffer; }
   bool inUse() const { return m_useCount > 0; }

   void incUse() { m_useCount++; }
   void decUse() { m_useCount--; }

   void copy( const void* pData, size_t offset, size_t size );

  private:
   void _mapMemory();
   void _unmapMemory();
   void _allocateMemory();

   const Device* m_pDevice = nullptr;

   // Used for staging buffers
   void* m_data = nullptr;

   // Common
   size_t m_size             = 0;
   VkBuffer m_vkBuffer       = nullptr;
   VkDeviceMemory m_vkMemory = nullptr;
   CYD::MemoryTypeFlag m_memoryType;

   uint32_t m_useCount = 0;
};
}
