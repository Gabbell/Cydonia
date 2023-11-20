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
FWDHANDLE( VkSwapchainKHR );

namespace vk
{
class Device;
class Swapchain;
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
   uint32_t getDepth() const noexcept { return m_type == CYD::ImageType::TEXTURE_2D ? m_depth : 1; }
   uint32_t getMipLevels() const noexcept { return m_mipLevels; }
   CYD::PixelFormat getPixelFormat() const noexcept { return m_format; }
   CYD::PipelineStageFlag getStages() const noexcept { return m_stages; }
   CYD::Access getPreviousAccess( uint32_t mipLevel = 0 ) const;
   const std::string& getName() const noexcept { return m_name; }

   VkImage getVKImage() const noexcept { return m_vkImage; }
   VkImageView getVKImageView() const noexcept { return m_vkImageView; }
   VkDeviceMemory getVKDeviceMemory() const noexcept { return m_vkMemory; }

   bool inUse() const { return ( *m_useCount ) > 0; }

   void setPreviousAccess( CYD::Access access, uint32_t mipLevel = 0 );

   void incUse();
   void decUse();

  private:
   void _createImage();
   void _allocateMemory();
   void _createImageView();

   const Device* m_pDevice = nullptr;

   // Texture description
   size_t m_size        = 0;
   uint32_t m_width     = 0;
   uint32_t m_height    = 0;
   uint32_t m_depth     = 1;  // For 3D images and cube maps
   uint32_t m_mipLevels = 1;

   CYD::ImageType m_type     = CYD::ImageType::TEXTURE_2D;
   CYD::PixelFormat m_format = CYD::PixelFormat::UNKNOWN;

   // TODO: This changes as we build command lists. It is not thread safe
   static constexpr uint32_t MAX_MIP_LEVEL = 16;
   std::array<CYD::Access, MAX_MIP_LEVEL> m_prevAccesses;

   CYD::ImageUsageFlag m_usage     = 0;
   CYD::PipelineStageFlag m_stages = 0;

   std::string m_name = "";

   VkImage m_vkImage         = nullptr;
   VkImageView m_vkImageView = nullptr;
   VkDeviceMemory m_vkMemory = nullptr;

   std::unique_ptr<std::atomic<uint32_t>> m_useCount;
};
}
