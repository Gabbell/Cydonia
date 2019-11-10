#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkBuffer );
FWDHANDLE( VkDeviceMemory );
FWDHANDLE( VkDescriptorSet );
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
   const VkDescriptorSet& getVKDescSet() const noexcept { return _vkDescSet; }

   void updateDescriptorSet( const ShaderObjectInfo& info, VkDescriptorSet descSet );
   void mapMemory( void* data, size_t size );

  private:
   void _allocateMemory();

   const Device& _device;

   // Used for staging buffers
   void* _data;

   // Common
   size_t _size;
   VkBuffer _vkBuffer       = nullptr;
   VkDeviceMemory _vkMemory = nullptr;
   BufferUsageFlag _usage;
   MemoryTypeFlag _memoryType;

   // Optional for shader accessible buffers
   VkDescriptorSet _vkDescSet = nullptr;
};
}
