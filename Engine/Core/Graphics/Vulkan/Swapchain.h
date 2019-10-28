#pragma once

#include <Core/Common/Include.h>

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
struct VkSurfaceFormatKHR;
struct VkExtent2D;
enum VkPresentModeKHR;

namespace cyd
{
class Device;
class Surface;
class CommandBuffer;
struct SwapchainInfo;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Swapchain
{
  public:
   Swapchain( Device& device, const Surface& surface, const SwapchainInfo& info );
   ~Swapchain();

   void initFramebuffers( VkRenderPass renderPass );
   void acquireImage( const CommandBuffer* buffer );
   void present();

   VkFramebuffer getCurrentFramebuffer() const;

   const VkSwapchainKHR& getVKSwapchain() const noexcept { return _vkSwapchain; }
   const VkExtent2D& getVKExtent() const noexcept { return *_extent; }
   const VkSurfaceFormatKHR& getFormat() const noexcept { return *_surfaceFormat; }

  private:
   void _createSwapchain( const SwapchainInfo& info );
   void _createImageViews();
   void _createSyncObjects();

   // Used to create the swapchain
   Device& _device;
   const Surface& _surface;

   VkRenderPass _prevRenderPass = nullptr;

   uint32_t _imageCount = 0;
   uint32_t _imageIndex = 0;
   std::vector<VkImageView> _imageViews;
   std::vector<VkImage> _images;
   std::vector<VkFramebuffer> _frameBuffers;

   uint32_t _currentFrame = 0;
   std::vector<VkSemaphore> _availableSems;
   std::vector<VkSemaphore> _renderDoneSems;
   const CommandBuffer* _inFlightCmdBuffer = nullptr;

   VkSwapchainKHR _vkSwapchain = nullptr;

   // Swapchain Properties
   VkPresentModeKHR _presentMode;                       // Presentation mode
   std::unique_ptr<VkSurfaceFormatKHR> _surfaceFormat;  // Swapchain image format
   std::unique_ptr<VkExtent2D> _extent;                 // Actual swapchain extent
};
}
