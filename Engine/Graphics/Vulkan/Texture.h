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
   CYD::PixelFormat getPixelFormat() const noexcept { return m_format; }
   CYD::PipelineStageFlag getStages() const noexcept { return m_stages; }
   CYD::Access getPreviousAccess() const noexcept { return m_prevAccess; }
   const std::string& getName() const noexcept { return m_name; }

   VkImage getVKImage() const noexcept { return m_vkImage; }
   VkImageView getVKImageView() const noexcept { return m_vkImageView; }
   VkDeviceMemory getVKDeviceMemory() const noexcept { return m_vkMemory; }

   bool inUse() const { return ( *m_useCount ) > 0; }

   void setPreviousAccess( CYD::Access access ) { m_prevAccess = access; }

   void incUse();
   void decUse();

  private:
   void _createImage();
   void _allocateMemory();
   void _createImageView();

   const Device* m_pDevice = nullptr;

   // Texture description
   size_t m_size     = 0;
   uint32_t m_width  = 0;
   uint32_t m_height = 0;
   uint32_t m_depth  = 1;  // For 3D images and cube maps

   CYD::ImageType m_type     = CYD::ImageType::TEXTURE_2D;
   CYD::PixelFormat m_format = CYD::PixelFormat::UNKNOWN;
   CYD::Access m_prevAccess  = CYD::Access::UNDEFINED;

   CYD::ImageUsageFlag m_usage     = 0;
   CYD::PipelineStageFlag m_stages = 0;

   std::string m_name = "";

   VkImage m_vkImage         = nullptr;
   VkImageView m_vkImageView = nullptr;
   VkDeviceMemory m_vkMemory = nullptr;

   std::unique_ptr<std::atomic<uint32_t>> m_useCount;
};
}
