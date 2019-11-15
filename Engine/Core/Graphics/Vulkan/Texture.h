#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

#include <cstdint>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkImage );
FWDHANDLE( VkImageView );
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
struct TextureDescription
{
   size_t size;
   uint32_t width;
   uint32_t height;
   ImageType type;
   PixelFormat format;
   ImageUsageFlag usage;
};

class Texture
{
  public:
   Texture( const Device& device, const TextureDescription& desc );
   ~Texture();

   size_t getSize() const noexcept { return _size; }
   uint32_t getWidth() const noexcept { return _width; }
   uint32_t getHeight() const noexcept { return _height; }
   ImageLayout getLayout() const noexcept { return _layout; }
   const VkImage& getVKImage() const noexcept { return _vkImage; }
   const VkDescriptorSet& getVKDescSet() const noexcept { return _vkDescSet; }

   void setLayout( ImageLayout layout ) { _layout = layout; }

   void updateDescriptorSet( const ShaderObjectInfo& info, VkDescriptorSet descSet );

  private:
   void _createImage();
   void _allocateMemory();
   void _createImageView();

   const Device& _device;

   size_t _size   = 0;
   uint32_t _width  = 0;
   uint32_t _height = 0;
   ImageType _type;
   PixelFormat _format;
   ImageUsageFlag _usage;
   ImageLayout _layout;

   VkImage _vkImage           = nullptr;
   VkImageView _vkImageView   = nullptr;
   VkDeviceMemory _vkMemory   = nullptr;
   VkDescriptorSet _vkDescSet = nullptr;
};
}
