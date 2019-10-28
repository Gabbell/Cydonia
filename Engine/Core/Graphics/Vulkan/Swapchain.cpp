#include <Core/Graphics/Vulkan/Swapchain.h>

#include <Core/Common/Vulkan.h>
#include <Core/Common/Assert.h>

#include <Core/Graphics/Vulkan/Types.h>
#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/Surface.h>
#include <Core/Graphics/Vulkan/CommandBuffer.h>

#include <algorithm>

// Double-buffered
static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

cyd::Swapchain::Swapchain( Device& device, const Surface& surface, const SwapchainInfo& info )
    : _device( device ), _surface( surface )
{
   _createSwapchain( info );
   _createImageViews();
   _createSyncObjects();
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
      VkExtent2D actualExtent = { extent.width, extent.height };

      actualExtent.width =
          std::clamp( actualExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width );
      actualExtent.height =
          std::clamp( actualExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height );

      return actualExtent;
   }
}

static VkSurfaceFormatKHR chooseFormat(
    cyd::PixelFormat format,
    cyd::ColorSpace space,
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

   VkSurfaceFormatKHR desiredFormat;

   auto it = std::find_if(
       formats.begin(), formats.end(), [&desiredFormat]( const VkSurfaceFormatKHR& format ) {
          return format.format == desiredFormat.format &&
                 format.colorSpace == desiredFormat.colorSpace;
       } );

   if( it != formats.end() )
   {
      return *it;
   }

   return formats[0];
}

static VkPresentModeKHR choosePresentMode(
    cyd::PresentMode mode,
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

   VkPresentModeKHR desiredMode;
   switch( mode )
   {
      case cyd::PresentMode::FIFO:
         desiredMode = VK_PRESENT_MODE_FIFO_KHR;
         break;
      case cyd::PresentMode::FIFO_RELAXED:
         desiredMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
         break;
      case cyd::PresentMode::IMMEDIATE:
         desiredMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
         break;
      case cyd::PresentMode::MAILBOX:
         desiredMode = VK_PRESENT_MODE_MAILBOX_KHR;
         break;
      default:
         CYDASSERT( !"Swapchain: Present mode not supported" );
   }

   auto it = std::find( presentModes.begin(), presentModes.end(), desiredMode );

   if( it != presentModes.end() )
   {
      return *it;
   }

   return presentModes[0];
}

void cyd::Swapchain::_createSwapchain( const SwapchainInfo& info )
{
   const VkDevice& vkDevice           = _device.getVKDevice();
   const VkPhysicalDevice& physDevice = _device.getPhysicalDevice();
   const VkSurfaceKHR& vkSurface      = _surface.getVKSurface();

   VkSurfaceCapabilitiesKHR caps;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physDevice, vkSurface, &caps );

   _surfaceFormat = std::make_unique<VkSurfaceFormatKHR>(
       chooseFormat( info.format, info.space, physDevice, vkSurface ) );
   _extent      = std::make_unique<VkExtent2D>( chooseExtent( info.extent, caps ) );
   _imageCount  = chooseImageCount( caps );
   _presentMode = choosePresentMode( info.mode, physDevice, vkSurface );

   VkSwapchainCreateInfoKHR createInfo = {};
   createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface                  = vkSurface;

   createInfo.minImageCount    = _imageCount;
   createInfo.imageFormat      = _surfaceFormat->format;
   createInfo.imageColorSpace  = _surfaceFormat->colorSpace;
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
   _images.resize( _imageCount );

   vkGetSwapchainImagesKHR( vkDevice, _vkSwapchain, &_imageCount, _images.data() );
}

void cyd::Swapchain::_createImageViews()
{
   _imageViews.resize( _imageCount );
   for( size_t i = 0; i < _imageCount; ++i )
   {
      VkImageViewCreateInfo createInfo           = {};
      createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image                           = _images[i];
      createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format                          = _surfaceFormat->format;
      createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel   = 0;
      createInfo.subresourceRange.levelCount     = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount     = 1;

      VkResult result =
          vkCreateImageView( _device.getVKDevice(), &createInfo, nullptr, &_imageViews[i] );
      CYDASSERT( result == VK_SUCCESS && "Swapchain: Could not create image views" );
   }
}

void cyd::Swapchain::_createSyncObjects()
{
   _availableSems.resize( MAX_FRAMES_IN_FLIGHT );
   _renderDoneSems.resize( MAX_FRAMES_IN_FLIGHT );

   VkSemaphoreCreateInfo semaphoreInfo = {};
   semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fenceInfo = {};
   fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

   for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
   {
      if( vkCreateSemaphore( _device.getVKDevice(), &semaphoreInfo, nullptr, &_availableSems[i] ) !=
              VK_SUCCESS ||
          vkCreateSemaphore(
              _device.getVKDevice(), &semaphoreInfo, nullptr, &_renderDoneSems[i] ) != VK_SUCCESS )
      {
         CYDASSERT( !"Swapchain: Could not create sync objects" );
      }
   }
}

void cyd::Swapchain::initFramebuffers( const VkRenderPass renderPass )
{
   if( ( renderPass && !_prevRenderPass ) ||
       ( renderPass && _prevRenderPass && ( renderPass != _prevRenderPass ) ) )
   {
      _prevRenderPass = renderPass;

      _frameBuffers.resize( _imageCount );
      for( size_t i = 0; i < _imageCount; i++ )
      {
         VkImageView attachments[] = { _imageViews[i] };

         VkFramebufferCreateInfo framebufferInfo = {};
         framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
         framebufferInfo.renderPass              = renderPass;
         framebufferInfo.attachmentCount         = 1;
         framebufferInfo.pAttachments            = attachments;
         framebufferInfo.width                   = _extent->width;
         framebufferInfo.height                  = _extent->height;
         framebufferInfo.layers                  = 1;

         VkResult result = vkCreateFramebuffer(
             _device.getVKDevice(), &framebufferInfo, nullptr, &_frameBuffers[i] );
         CYDASSERT( result == VK_SUCCESS && "Swapchain: Could not create framebuffer" );
      }
   }
}

void cyd::Swapchain::acquireImage( const CommandBuffer* buffer )
{
   if( _inFlightCmdBuffer )
   {
      vkWaitForFences(
          _device.getVKDevice(), 1, &_inFlightCmdBuffer->getVKFence(), VK_TRUE, UINT64_MAX );
   }

   vkAcquireNextImageKHR(
       _device.getVKDevice(),
       _vkSwapchain,
       UINT64_MAX,
       _availableSems[_currentFrame],
       VK_NULL_HANDLE,
       &_imageIndex );

   _inFlightCmdBuffer = buffer;
}

void cyd::Swapchain::present()
{
   if( _inFlightCmdBuffer )
   {
      const VkCommandBuffer& currentCmdBuffer = _inFlightCmdBuffer->getVKBuffer();
      const VkFence& currentFence             = _inFlightCmdBuffer->getVKFence();

      VkSubmitInfo submitInfo = {};
      submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      VkSemaphore waitSemaphores[]      = { _availableSems[_currentFrame] };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      submitInfo.waitSemaphoreCount     = 1;
      submitInfo.pWaitSemaphores        = waitSemaphores;
      submitInfo.pWaitDstStageMask      = waitStages;

      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers    = &currentCmdBuffer;

      VkSemaphore signalSemaphores[]  = { _renderDoneSems[_currentFrame] };
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores    = signalSemaphores;

      // FIXME NO HARDCODING 0
      const VkQueue* graphicsPresentQueue = _device.getQueue( 0, true );
      if( graphicsPresentQueue )
      {
         vkQueueSubmit( *graphicsPresentQueue, 1, &submitInfo, currentFence );

         VkPresentInfoKHR presentInfo = {};
         presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

         presentInfo.waitSemaphoreCount = 1;
         presentInfo.pWaitSemaphores    = signalSemaphores;

         VkSwapchainKHR swapChains[] = { _vkSwapchain };
         presentInfo.swapchainCount  = 1;
         presentInfo.pSwapchains     = swapChains;

         presentInfo.pImageIndices = &_imageIndex;

         vkQueuePresentKHR( *graphicsPresentQueue, &presentInfo );

         _currentFrame = ( _currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
      }
      else
      {
         CYDASSERT( !"Swapchain: Could not find graphics presentation queue" );
      }
   }
   else
   {
      CYDASSERT( !"Swapchain: No in flight command buffer. Was begin pass called?" );
   }
}

VkFramebuffer cyd::Swapchain::getCurrentFramebuffer() const { return _frameBuffers[_currentFrame]; }

cyd::Swapchain::~Swapchain()
{
   for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
   {
      vkDestroySemaphore( _device.getVKDevice(), _renderDoneSems[i], nullptr );
      vkDestroySemaphore( _device.getVKDevice(), _availableSems[i], nullptr );
   }

   for( auto frameBuffer : _frameBuffers )
   {
      vkDestroyFramebuffer( _device.getVKDevice(), frameBuffer, nullptr );
   }

   for( auto imageView : _imageViews )
   {
      vkDestroyImageView( _device.getVKDevice(), imageView, nullptr );
   }

   vkDestroySwapchainKHR( _device.getVKDevice(), _vkSwapchain, nullptr );
}
