#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Vulkan/VulkanTypes.h>
#include <Graphics/Vulkan.h>

#include <cstdint>
#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSwapchainKHR );
FWDHANDLE( VkImage );
FWDHANDLE( VkImageView );
FWDHANDLE( VkFramebuffer );
FWDHANDLE( VkRenderPass );
FWDHANDLE( VkSemaphore );
FWDHANDLE( VkDeviceMemory );
FWDHANDLE( VkFence );
struct VkSurfaceFormatKHR;
struct VkExtent2D;

namespace vk
{
class Device;
class Surface;
class Texture;
class CommandBuffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Swapchain final
{
  public:
   Swapchain( Device& device, const Surface& surface, const CYD::SwapchainInfo& info );
   NON_COPIABLE( Swapchain );
   ~Swapchain();

   // Double-buffered
   static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

   bool isReady() const { return m_ready; }

   // For render pass begin info
   const VkExtent2D& getVKExtent() const { return m_extent; }
   uint32_t getWidth() const { return m_extent.width; }
   uint32_t getHeight() const { return m_extent.height; }

   const RenderPassInfo& getRenderPass() const { return m_renderPasses[m_shouldClear ? 0 : 1]; }

   VkFramebuffer getCurrentVKFramebuffer() const { return m_vkFramebuffers[m_currentFrame]; }
   VkRenderPass getVKRenderPass() const { return m_vkRenderPasses[m_shouldClear ? 0 : 1]; }
   VkSwapchainKHR getVKSwapchain() const noexcept { return m_vkSwapchain; }
   VkSemaphore getSemToWait() const noexcept { return m_availableSems[m_currentFrame]; }
   VkSemaphore getSemToSignal() const noexcept { return m_renderDoneSems[m_currentFrame]; }

   VkImage getColorVKImage() const { return m_colorImages[m_currentFrame]; }
   VkImageView getColorVKImageView() const { return m_colorImageViews[m_currentFrame]; }
   CYD::Access getColorVKImageAccess() const { return m_colorImageAccess[m_currentFrame]; }

   const CYD::PixelFormat getPixelFormat() const noexcept { return m_pixelFormat; }

   uint32_t getImageCount() const { return m_imageCount; }
   uint32_t getCurrentFrame() const { return m_currentFrame; }

   void setClear( bool shouldClear ) { m_shouldClear = shouldClear; }
   void transitionColorImage( const CommandBuffer* cmdBuffer, CYD::Access nextAccess );

   bool acquireImage();
   void present();
   void recreateSwapchain( const CYD::SwapchainInfo& info );

  private:
   void _createSwapchain( const CYD::SwapchainInfo& info );
   void _cleanupSwapchain();

   void _createFramebuffers();

   void _createRenderPasses();
   void _createSyncObjects();

   // Used to create the swapchain
   Device& m_device;
   const Surface& m_surface;

   uint32_t m_imageCount = 0;
   uint32_t m_imageIndex = 0;

   CYD::PixelFormat m_pixelFormat;

   std::vector<VkImage> m_colorImages;
   std::vector<VkImageView> m_colorImageViews;
   std::vector<CYD::Access> m_colorImageAccess;

   std::vector<VkFramebuffer> m_vkFramebuffers;

   RenderPassInfo m_renderPasses[2];
   VkRenderPass m_vkRenderPasses[2];
   bool m_shouldClear = true;

   bool m_ready            = false;  // Used to determine if we acquired an image before presenting
   uint32_t m_currentFrame = 0;
   std::vector<VkSemaphore> m_availableSems;
   std::vector<VkSemaphore> m_renderDoneSems;

   VkSwapchainKHR m_vkSwapchain = nullptr;

   // Swapchain Properties
   VkPresentModeKHR m_presentMode;                       // Presentation mode
   VkSurfaceFormatKHR m_surfaceFormat;  // Swapchain image format
   VkExtent2D m_extent;                 // Actual swapchain extent
};
}
