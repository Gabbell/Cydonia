#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkBuffer );
FWDHANDLE( VkDeviceMemory );
namespace cyd
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Buffer
{
  public:
   Buffer( const Device& device, size_t size, BufferUsageFlag usage, MemoryTypeFlag memoryType );
   ~Buffer();

   size_t getSize() const noexcept { return _size; }
   VkBuffer getVKBuffer() const noexcept { return _vkBuffer; }

   void mapMemory( void* data, size_t size );

  private:
   void _allocateMemory();

   const Device& _device;

   // Used for staging
   void* _data;

   size_t _size;
   VkBuffer _vkBuffer;
   VkDeviceMemory _vkMemory;
   BufferUsageFlag _usage;
   MemoryTypeFlag _memoryType;
};
}
