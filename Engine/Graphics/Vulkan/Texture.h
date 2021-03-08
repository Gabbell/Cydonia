#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <atomic>
#include <cstdint>
#include <memory>

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
   Texture();
   MOVABLE( Texture );
   ~Texture();

   void acquire( const Device& device, const CYD::TextureDescription& desc );
   void release();

   size_t getSize() const noexcept { return m_size; }
   uint32_t getWidth() const noexcept { return m_width; }
   uint32_t getHeight() const noexcept { return m_height; }
   uint32_t getLayers() const noexcept { return m_layers; }
   CYD::PixelFormat getPixelFormat() const noexcept { return m_format; }
   CYD::ShaderStageFlag getStages() const noexcept { return m_stages; }

   CYD::ImageLayout getLayout() const noexcept { return m_layout; }
   void setLayout( CYD::ImageLayout layout ) { m_layout = layout; }

   const VkImage& getVKImage() const noexcept { return m_vkImage; }
   const VkImageView& getVKImageView() const noexcept { return m_vkImageView; }
   bool inUse() const { return ( *m_useCount ) > 0; }

   void incUse() { ( *m_useCount )++; }
   void decUse() { ( *m_useCount )--; }

  private:
   void _createImage();
   void _allocateMemory();
   void _createImageView();

   const Device* m_pDevice = nullptr;

   // Texture description
   size_t m_size                 = 0;
   uint32_t m_width              = 0;
   uint32_t m_height             = 0;
   uint32_t m_layers             = 1;  // For 3D images and cube maps
   CYD::ImageType m_type         = CYD::ImageType::TEXTURE_2D;
   CYD::PixelFormat m_format     = CYD::PixelFormat::BGRA8_UNORM;
   CYD::ImageLayout m_layout     = CYD::ImageLayout::UNKNOWN;
   CYD::ImageUsageFlag m_usage   = 0;
   CYD::ShaderStageFlag m_stages = 0;

   VkImage m_vkImage         = nullptr;
   VkImageView m_vkImageView = nullptr;
   VkDeviceMemory m_vkMemory = nullptr;

   std::unique_ptr<std::atomic<uint32_t>> m_useCount;
};
}
