#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <cstdint>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkImage );
FWDHANDLE( VkImageView );
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
class Texture final
{
  public:
   Texture() = default;
   MOVABLE( Texture );
   ~Texture() = default;

   void seize( const Device& device, const cyd::TextureDescription& desc );
   void release();

   size_t getSize() const noexcept { return _size; }
   uint32_t getWidth() const noexcept { return _width; }
   uint32_t getHeight() const noexcept { return _height; }
   cyd::ImageLayout getLayout() const noexcept { return _layout; }
   const VkImage& getVKImage() const noexcept { return _vkImage; }
   const VkDescriptorSet& getVKDescSet() const noexcept { return _vkDescSet; }
   bool inUse() const { return _inUse; }

   void setUnused() { _inUse = false; }
   void setLayout( cyd::ImageLayout layout ) { _layout = layout; }

   void updateDescriptorSet( const cyd::ShaderObjectInfo& info, VkDescriptorSet descSet );

  private:
   void _createImage();
   void _allocateMemory();
   void _createImageView();

   const Device* _device = nullptr;

   size_t _size     = 0;
   uint32_t _width  = 0;
   uint32_t _height = 0;
   cyd::ImageType _type;
   cyd::PixelFormat _format;
   cyd::ImageUsageFlag _usage;
   cyd::ImageLayout _layout;

   VkImage _vkImage           = nullptr;
   VkImageView _vkImageView   = nullptr;
   VkDeviceMemory _vkMemory   = nullptr;
   VkDescriptorSet _vkDescSet = nullptr;

   bool _inUse = false;
};
}
