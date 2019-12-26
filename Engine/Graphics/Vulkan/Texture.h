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

   void acquire( const Device& device, const cyd::TextureDescription& desc );
   void release();

   size_t getSize() const noexcept { return m_size; }
   uint32_t getWidth() const noexcept { return m_width; }
   uint32_t getHeight() const noexcept { return m_height; }
   cyd::ImageLayout getLayout() const noexcept { return m_layout; }
   const VkImage& getVKImage() const noexcept { return m_vkImage; }
   const VkDescriptorSet& getVKDescSet() const noexcept { return m_vkDescSet; }
   bool inUse() const { return m_inUse; }

   void setUnused() { m_inUse = false; }
   void setLayout( cyd::ImageLayout layout ) { m_layout = layout; }

   void updateDescriptorSet( const cyd::ShaderObjectInfo& info, VkDescriptorSet descSet );

  private:
   void _createImage();
   void _allocateMemory();
   void _createImageView();

   const Device* m_pDevice = nullptr;

   size_t m_size     = 0;
   uint32_t m_width  = 0;
   uint32_t m_height = 0;
   cyd::ImageType m_type;
   cyd::PixelFormat m_format;
   cyd::ImageUsageFlag m_usage;
   cyd::ImageLayout m_layout;

   VkImage m_vkImage           = nullptr;
   VkImageView m_vkImageView   = nullptr;
   VkDeviceMemory m_vkMemory   = nullptr;
   VkDescriptorSet m_vkDescSet = nullptr;

   bool m_inUse = false;
};
}
