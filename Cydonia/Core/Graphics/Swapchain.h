#pragma once

#include <Core/Common/Common.h>

#include <cstdint>
#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSwapchainKHR );
FWDHANDLE( VkImage );
struct VkSurfaceFormatKHR;
struct VkExtent2D;
enum VkPresentModeKHR;

namespace cyd
{
class Device;
class Surface;
struct Extent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Swapchain
{
  public:
   Swapchain( const Device& device, const Surface& surface, const Extent& windowExtent );
   ~Swapchain();

   const VkSwapchainKHR& getVKSwapchain() const noexcept { return _vkSwapchain; }

  private:
   void _createSwapchain();

   // Used to create the swapchain
   const Device& _device;
   const Surface& _surface;
   const Extent& _windowExtent;

   // Swapchain Properties
   // =============================================================================================
   VkSwapchainKHR _vkSwapchain = nullptr;
   uint32_t _imageCount;
   std::vector<VkImage> _swapImages;
   VkPresentModeKHR _presentMode;                // Presentation mode
   std::unique_ptr<VkSurfaceFormatKHR> _format;  // Swapchain image format
   std::unique_ptr<VkExtent2D> _extent;          // Actual swapchain extent
};
}
