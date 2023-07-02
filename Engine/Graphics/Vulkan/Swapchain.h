#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

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
enum VkPresentModeKHR;

namespace vk
{
class Device;
class Surface;
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
   const VkExtent2D& getVKExtent() const { return *m_extent; }
   VkFramebuffer getCurrentFramebuffer() const { return m_frameBuffers[m_currentFrame]; }
   VkRenderPass getRenderPass() const;

   const VkSwapchainKHR& getVKSwapchain() const noexcept { return m_vkSwapchain; }
   const VkSurfaceFormatKHR& getFormat() const noexcept { return *m_surfaceFormat; }
   const VkSemaphore& getSemToWait() const noexcept { return m_availableSems[m_currentFrame]; }
   const VkSemaphore& getSemToSignal() const noexcept { return m_renderDoneSems[m_currentFrame]; }

   uint32_t getImageCount() const { return m_imageCount; }
   uint32_t getCurrentFrame() const { return m_currentFrame; }

   void setToLoad() { m_shouldLoad = true; }

   void acquireImage();
   void present();

  private:
   void _createSwapchain( const CYD::SwapchainInfo& info );
   void _createImageViews();
   void _createDepthResources();
   void _createSyncObjects();
   void _createFramebuffers( const CYD::SwapchainInfo& info );

   // Used to create the swapchain
   Device& m_device;
   const Surface& m_surface;

   VkRenderPass m_clearRenderPass = nullptr;
   VkRenderPass m_loadRenderPass  = nullptr;

   uint32_t m_imageCount = 0;
   uint32_t m_imageIndex = 0;
   std::vector<VkImageView> m_imageViews;
   std::vector<VkImage> m_images;

   std::vector<VkFramebuffer> m_frameBuffers;

   VkImageView m_depthImageView;
   VkImage m_depthImage;
   VkDeviceMemory m_depthImageMemory;

   bool m_ready            = false;  // Used to determine if we acquired an image before presenting
   bool m_shouldLoad       = false;  // Used to determine which render pass to return
   uint32_t m_currentFrame = 0;
   std::vector<VkSemaphore> m_availableSems;
   std::vector<VkSemaphore> m_renderDoneSems;

   VkSwapchainKHR m_vkSwapchain = nullptr;

   // Swapchain Properties
   VkPresentModeKHR m_presentMode;                       // Presentation mode
   std::unique_ptr<VkSurfaceFormatKHR> m_surfaceFormat;  // Swapchain image format
   std::unique_ptr<VkExtent2D> m_extent;                 // Actual swapchain extent
};
}
