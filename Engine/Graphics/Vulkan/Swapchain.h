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
struct VkSurfaceFormatKHR;
struct VkExtent2D;
enum VkPresentModeKHR;

namespace vk
{
class Device;
class Surface;
class CommandBuffer;
struct RenderPassInfo;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Swapchain final
{
  public:
   Swapchain( Device& device, const Surface& surface, const cyd::SwapchainInfo& info );
   ~Swapchain();

   void initFramebuffers( const cyd::RenderPassInfo& info, VkRenderPass renderPass );
   void acquireImage( const CommandBuffer* buffer );
   void present();

   VkFramebuffer getCurrentFramebuffer() const;

   const VkSwapchainKHR& getVKSwapchain() const noexcept { return m_vkSwapchain; }
   const VkExtent2D& getVKExtent() const noexcept { return *m_extent; }
   const VkSurfaceFormatKHR& getFormat() const noexcept { return *m_surfaceFormat; }
   const VkSemaphore& getSemToWait() const noexcept { return m_availableSems[m_currentFrame]; }
   const VkSemaphore& getSemToSignal() const noexcept { return m_renderDoneSems[m_currentFrame]; }

  private:
   void _createSwapchain( const cyd::SwapchainInfo& info );
   void _createImageViews();
   void _createDepthResources();
   void _createSyncObjects();

   // Used to create the swapchain
   Device& m_device;
   const Surface& m_surface;

   VkRenderPass m_prevRenderPass = nullptr;

   uint32_t m_imageCount = 0;
   uint32_t m_imageIndex = 0;
   std::vector<VkImageView> m_imageViews;
   std::vector<VkImage> m_images;
   std::vector<VkFramebuffer> m_frameBuffers;
   VkImageView m_depthImageView;
   VkImage m_depthImage;
   VkDeviceMemory m_depthImageMemory;

   uint32_t m_currentFrame = 0;
   std::vector<VkSemaphore> m_availableSems;
   std::vector<VkSemaphore> m_renderDoneSems;
   const CommandBuffer* m_inFlightCmdBuffer = nullptr;

   VkSwapchainKHR m_vkSwapchain = nullptr;

   // Swapchain Properties
   VkPresentModeKHR m_presentMode;                       // Presentation mode
   std::unique_ptr<VkSurfaceFormatKHR> m_surfaceFormat;  // Swapchain image format
   std::unique_ptr<VkExtent2D> m_extent;                 // Actual swapchain extent
};
}
