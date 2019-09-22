#include <Core/Graphics/Swapchain.h>

#include <Core/Common/Vulkan.h>
#include <Core/Common/Assert.h>

#include <Core/Graphics/CommonTypes.h>
#include <Core/Graphics/Device.h>
#include <Core/Graphics/Surface.h>
#include <Core/Window/Window.h>

cyd::Swapchain::Swapchain(
    const Device& device,
    const Surface& surface,
    const Extent& windowExtent )
    : _device( device ), _surface( surface ), _windowExtent( windowExtent )
{
   _createSwapchain();
}

static uint32_t chooseImageCount( const VkSurfaceCapabilitiesKHR& caps )
{
   uint32_t imageCount = caps.minImageCount;

   if( caps.maxImageCount > 0 && imageCount > caps.maxImageCount )
   {
      imageCount = caps.maxImageCount;
   }

   return imageCount;
}

static VkExtent2D chooseExtent( const cyd::Extent& extent, const VkSurfaceCapabilitiesKHR& caps )
{
   if( caps.currentExtent.width != UINT32_MAX )
   {
      return caps.currentExtent;
   }
   else
   {
      // Use the window extent
      VkExtent2D actualExtent = {extent.width, extent.height};

      actualExtent.width =
          std::clamp( actualExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width );
      actualExtent.height =
          std::clamp( actualExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height );

      return actualExtent;
   }
}

static VkSurfaceFormatKHR chooseFormat(
    const VkPhysicalDevice& physDevice,
    const VkSurfaceKHR& vkSurface )
{
   std::vector<VkSurfaceFormatKHR> formats;

   uint32_t formatCount;
   vkGetPhysicalDeviceSurfaceFormatsKHR( physDevice, vkSurface, &formatCount, nullptr );

   if( formatCount > 0 )
   {
      formats.resize( formatCount );
      vkGetPhysicalDeviceSurfaceFormatsKHR( physDevice, vkSurface, &formatCount, formats.data() );
   }

   auto it = std::find_if( formats.begin(), formats.end(), []( const VkSurfaceFormatKHR& format ) {
      return format.format == VK_FORMAT_B8G8R8A8_UNORM &&
             format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
   } );

   if( it != formats.end() )
   {
      return *it;
   }

   return formats[0];
}

static VkPresentModeKHR choosePresentMode(
    const VkPhysicalDevice& physDevice,
    const VkSurfaceKHR& vkSurface )
{
   std::vector<VkPresentModeKHR> presentModes;

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR( physDevice, vkSurface, &presentModeCount, nullptr );

   if( presentModeCount != 0 )
   {
      presentModes.resize( presentModeCount );
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          physDevice, vkSurface, &presentModeCount, presentModes.data() );
   }

   auto it = std::find( presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR );

   if( it != presentModes.end() )
   {
      return *it;
   }

   return presentModes[0];
}

void cyd::Swapchain::_createSwapchain()
{
   const VkDevice& vkDevice           = _device.getVKDevice();
   const VkPhysicalDevice& physDevice = _device.getPhysicalDevice();
   const VkSurfaceKHR& vkSurface      = _surface.getVKSurface();

   VkSurfaceCapabilitiesKHR caps;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physDevice, vkSurface, &caps );

   _imageCount  = chooseImageCount( caps );
   _presentMode = choosePresentMode( physDevice, vkSurface );
   _extent      = std::make_unique<VkExtent2D>( chooseExtent( _windowExtent, caps ) );
   _format      = std::make_unique<VkSurfaceFormatKHR>( chooseFormat( physDevice, vkSurface ) );

   VkSwapchainCreateInfoKHR createInfo = {};
   createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface                  = vkSurface;

   createInfo.minImageCount    = _imageCount;
   createInfo.imageFormat      = _format->format;
   createInfo.imageColorSpace  = _format->colorSpace;
   createInfo.imageExtent      = *_extent;
   createInfo.imageArrayLayers = 1;
   createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   // TODO Sharing mode concurrent
   createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

   createInfo.preTransform   = caps.currentTransform;
   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   createInfo.presentMode    = _presentMode;
   createInfo.clipped        = VK_TRUE;

   createInfo.oldSwapchain = VK_NULL_HANDLE;

   VkResult result = vkCreateSwapchainKHR( vkDevice, &createInfo, nullptr, &_vkSwapchain );
   CYDASSERT( result == VK_SUCCESS && "Swapchain: Could not create swapchain" );

   vkGetSwapchainImagesKHR( vkDevice, _vkSwapchain, &_imageCount, nullptr );
   _swapImages.resize( _imageCount );

   vkGetSwapchainImagesKHR( vkDevice, _vkSwapchain, &_imageCount, _swapImages.data() );
}
cyd::Swapchain::~Swapchain() {}
