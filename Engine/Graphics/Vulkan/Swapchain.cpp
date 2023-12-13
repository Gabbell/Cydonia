#include <Graphics/Vulkan/Swapchain.h>

#include <Common/Assert.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Synchronization.h>
#include <Graphics/Vulkan/RenderPassCache.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <Profiling.h>

#include <algorithm>

namespace vk
{
Swapchain::Swapchain( Device& device, const Surface& surface, const CYD::SwapchainInfo& info )
    : m_device( device ), m_surface( surface )
{
   _createSwapchain( info );
   _createRenderPasses();
   _createFramebuffers();
   _createSyncObjects();
}

static uint32_t chooseImageCount( uint32_t desiredCount, const VkSurfaceCapabilitiesKHR& caps )
{
   return std::clamp( desiredCount, caps.minImageCount, caps.maxImageCount );
}

static VkExtent2D chooseExtent( const CYD::Extent2D& extent, const VkSurfaceCapabilitiesKHR& caps )
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
    CYD::PixelFormat format,
    CYD::ColorSpace space,
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
   desiredFormat.format     = TypeConversions::cydToVkFormat( format );
   desiredFormat.colorSpace = TypeConversions::cydToVkSpace( space );

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
    CYD::PresentMode mode,
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
      case CYD::PresentMode::FIFO:
         desiredMode = VK_PRESENT_MODE_FIFO_KHR;
         break;
      case CYD::PresentMode::FIFO_RELAXED:
         desiredMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
         break;
      case CYD::PresentMode::IMMEDIATE:
         desiredMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
         break;
      case CYD::PresentMode::MAILBOX:
         desiredMode = VK_PRESENT_MODE_MAILBOX_KHR;
         break;
      default:
         CYD_ASSERT( !"Swapchain: Present mode not supported" );
   }

   auto it = std::find( presentModes.begin(), presentModes.end(), desiredMode );

   if( it != presentModes.end() )
   {
      return *it;
   }

   return presentModes[0];
}

void Swapchain::_createSwapchain( const CYD::SwapchainInfo& info )
{
   const VkDevice& vkDevice           = m_device.getVKDevice();
   const VkPhysicalDevice& physDevice = m_device.getPhysicalDevice();
   const VkSurfaceKHR& vkSurface      = m_surface.getVKSurface();

   VkSurfaceCapabilitiesKHR caps;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physDevice, vkSurface, &caps );

   m_surfaceFormat = chooseFormat( info.format, info.space, physDevice, vkSurface );
   m_pixelFormat   = TypeConversions::vkToCydFormat( m_surfaceFormat.format );

   m_extent      = chooseExtent( info.extent, caps );
   m_imageCount  = chooseImageCount( info.imageCount, caps );
   m_presentMode = choosePresentMode( info.mode, physDevice, vkSurface );

   VkSwapchainCreateInfoKHR createInfo = {};
   createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface                  = vkSurface;

   createInfo.minImageCount    = m_imageCount;
   createInfo.imageFormat      = m_surfaceFormat.format;
   createInfo.imageColorSpace  = m_surfaceFormat.colorSpace;
   createInfo.imageExtent      = m_extent;
   createInfo.imageArrayLayers = 1;
   createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

   // TODO Sharing mode concurrent
   createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

   createInfo.preTransform   = caps.currentTransform;
   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   createInfo.presentMode    = m_presentMode;
   createInfo.clipped        = VK_TRUE;

   createInfo.oldSwapchain = VK_NULL_HANDLE;

   VkResult result = vkCreateSwapchainKHR( vkDevice, &createInfo, nullptr, &m_vkSwapchain );
   CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not create swapchain" );
}

void Swapchain::_cleanupSwapchain()
{
   for( uint32_t i = 0; i < m_imageCount; ++i )
   {
      vkDestroyFramebuffer( m_device.getVKDevice(), m_vkFramebuffers[i], nullptr );
      m_vkFramebuffers[i] = nullptr;

      // The color VkImages are managed by the VkSwapchainKHR object, only need to free the rest
      vkDestroyImageView( m_device.getVKDevice(), m_colorImageViews[i], nullptr );
      m_colorImageViews[i] = nullptr;
   }

   CYD_ASSERT( m_renderDoneSems.size() == m_availableSems.size() );
   for( uint32_t i = 0; i < m_renderDoneSems.size(); i++ )
   {
      vkDestroySemaphore( m_device.getVKDevice(), m_renderDoneSems[i], nullptr );
      vkDestroySemaphore( m_device.getVKDevice(), m_availableSems[i], nullptr );

      m_renderDoneSems[i] = nullptr;
      m_availableSems[i]  = nullptr;
   }

   vkDestroySwapchainKHR( m_device.getVKDevice(), m_vkSwapchain, nullptr );
   m_vkSwapchain = nullptr;

   m_shouldClear  = true;
   m_currentFrame = 0;
   m_imageCount   = 0;
   m_imageIndex   = 0;
   m_ready        = false;
}

void Swapchain::_createRenderPasses()
{
   // Initializing main render passes
   Attachment colorAttachment;
   colorAttachment.type          = CYD::AttachmentType::COLOR_PRESENTATION;
   colorAttachment.format        = m_pixelFormat;
   colorAttachment.loadOp        = CYD::LoadOp::CLEAR;
   colorAttachment.storeOp       = CYD::StoreOp::STORE;
   colorAttachment.clear.color   = { { 0.0f, 0.0f, 0.0f, 1.0f } };
   colorAttachment.initialAccess = CYD::Access::PRESENT;
   colorAttachment.nextAccess    = CYD::Access::PRESENT;

   RenderPassInfo& clearPass = m_renderPasses[0];
   clearPass.attachments.push_back( colorAttachment );

   colorAttachment.loadOp        = CYD::LoadOp::LOAD;
   colorAttachment.initialAccess = CYD::Access::PRESENT;

   RenderPassInfo& loadPass = m_renderPasses[1];
   loadPass.attachments.push_back( colorAttachment );

   m_vkRenderPasses[0] = m_device.getRenderPassCache().findOrCreate( clearPass );
   m_vkRenderPasses[1] = m_device.getRenderPassCache().findOrCreate( loadPass );
}

void Swapchain::_createFramebuffers()
{
   VkResult result;

   vkGetSwapchainImagesKHR( m_device.getVKDevice(), m_vkSwapchain, &m_imageCount, nullptr );
   m_colorImages.resize( m_imageCount );
   m_colorImageViews.resize( m_imageCount );
   m_colorImageAccess.resize( m_imageCount );
   m_vkFramebuffers.resize( m_imageCount );

   vkGetSwapchainImagesKHR(
       m_device.getVKDevice(), m_vkSwapchain, &m_imageCount, m_colorImages.data() );

   for( size_t i = 0; i < m_imageCount; ++i )
   {
      m_colorImageAccess[i] = CYD::Access::UNDEFINED;

      VkImageViewCreateInfo colorViewInfo           = {};
      colorViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      colorViewInfo.image                           = m_colorImages[i];
      colorViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      colorViewInfo.format                          = m_surfaceFormat.format;
      colorViewInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      colorViewInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      colorViewInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      colorViewInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      colorViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      colorViewInfo.subresourceRange.baseMipLevel   = 0;
      colorViewInfo.subresourceRange.levelCount     = 1;
      colorViewInfo.subresourceRange.baseArrayLayer = 0;
      colorViewInfo.subresourceRange.layerCount     = 1;

      result = vkCreateImageView(
          m_device.getVKDevice(), &colorViewInfo, nullptr, &m_colorImageViews[i] );
      CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not create image views" );

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass              = m_vkRenderPasses[1];  // Load
      framebufferInfo.attachmentCount         = 1;
      framebufferInfo.pAttachments            = &m_colorImageViews[i];
      framebufferInfo.width                   = m_extent.width;
      framebufferInfo.height                  = m_extent.height;
      framebufferInfo.layers                  = 1;

      VkResult result = vkCreateFramebuffer(
          m_device.getVKDevice(), &framebufferInfo, nullptr, &m_vkFramebuffers[i] );
      CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not create framebuffer" );
   }
}

void Swapchain::_createSyncObjects()
{
   VkSemaphoreCreateInfo semaphoreInfo;
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   semaphoreInfo.pNext = nullptr;
   semaphoreInfo.flags = 0;

   VkResult result;

   m_availableSems.resize( MAX_FRAMES_IN_FLIGHT );
   for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
   {
      result =
          vkCreateSemaphore( m_device.getVKDevice(), &semaphoreInfo, nullptr, &m_availableSems[i] );
      CYD_ASSERT( result == VK_SUCCESS );
   }

   m_renderDoneSems.resize( MAX_FRAMES_IN_FLIGHT );
   for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
   {
      result = vkCreateSemaphore(
          m_device.getVKDevice(), &semaphoreInfo, nullptr, &m_renderDoneSems[i] );
      CYD_ASSERT( result == VK_SUCCESS );
   }
}

void Swapchain::transitionColorImage( const CommandBuffer* cmdBuffer, CYD::Access nextAccess )
{
   Synchronization::ImageMemory(
       cmdBuffer->getVKCmdBuffer(),
       getColorVKImage(),
       1,
       m_surfaceFormat.format,
       getColorVKImageAccess(),
       nextAccess );

   m_colorImageAccess[m_currentFrame] = nextAccess;
}

bool Swapchain::acquireImage()
{
   CYD_TRACE();

   VkResult result = vkAcquireNextImageKHR(
       m_device.getVKDevice(),
       m_vkSwapchain,
       UINT64_MAX,
       m_availableSems[m_currentFrame],
       VK_NULL_HANDLE,
       &m_imageIndex );

   if( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
   {
      m_ready = false;
   }
   else
   {
      m_ready = true;
   }

   return m_ready;
}

void Swapchain::present()
{
   CYD_ASSERT(
       m_ready &&
       "Swapchain: No image was acquired before presenting. Are you rendering to the "
       "swapchain?" );

   // Before presenting, make sure the color image is in the proper layout

   const VkQueue presentQueue = m_device.getQueueFromUsage( CYD::QueueUsage::GRAPHICS, true );
   if( presentQueue )
   {
      VkSwapchainKHR swapChains[]  = { m_vkSwapchain };
      VkSemaphore waitSemaphores[] = { m_renderDoneSems[m_currentFrame] };

      VkPresentInfoKHR presentInfo   = {};
      presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores    = waitSemaphores;
      presentInfo.swapchainCount     = 1;
      presentInfo.pSwapchains        = swapChains;
      presentInfo.pImageIndices      = &m_imageIndex;

      vkQueuePresentKHR( presentQueue, &presentInfo );
      m_currentFrame = ( m_currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;

      m_ready = false;
   }
   else
   {
      CYD_ASSERT( !"Swapchain: Could not get a present queue" );
   }
}

void Swapchain::recreateSwapchain( const CYD::SwapchainInfo& info )
{
   _cleanupSwapchain();
   _createSwapchain( info );
   _createFramebuffers();
   _createSyncObjects();
}

Swapchain::~Swapchain() { _cleanupSwapchain(); }
}
