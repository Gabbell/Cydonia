#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

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
   MOVABLE( Buffer );
   ~Buffer() = default;

   void seize(
       const Device& device,
       size_t size,
       cyd::BufferUsageFlag usage,
       cyd::MemoryTypeFlag memoryType );
   void release();

   size_t getSize() const noexcept { return _size; }
   VkBuffer getVKBuffer() const noexcept { return _vkBuffer; }
   const VkDescriptorSet& getVKDescSet() const noexcept { return _vkDescSet; }
   bool inUse() const { return _inUse; }

   void setUnused() { _inUse = false; }

   void
   updateDescriptorSet( const cyd::ShaderObjectInfo& info, VkDescriptorSet descSet );
   void mapMemory( const void* data );

  private:
   void _allocateMemory();

   const Device* _device = nullptr;

   // Used for staging buffers
   void* _data = nullptr;

   // Common
   size_t _size             = 0;
   VkBuffer _vkBuffer       = nullptr;
   VkDeviceMemory _vkMemory = nullptr;
   cyd::MemoryTypeFlag _memoryType;

   // Optional for shader accessible buffers
   VkDescriptorSet _vkDescSet = nullptr;
   
   bool _inUse = false;
};
}
