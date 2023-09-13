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
       formats.begin(),
       formats.end(),
       [&desiredFormat]( const VkSurfaceFormatKHR& format )
       {
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

   m_surfaceFormat = std::make_unique<VkSurfaceFormatKHR>(
       chooseFormat( info.format, info.space, physDevice, vkSurface ) );
   m_extent      = std::make_unique<VkExtent2D>( chooseExtent( info.extent, caps ) );
   m_imageCount  = chooseImageCount( info.imageCount, caps );
   m_presentMode = choosePresentMode( info.mode, physDevice, vkSurface );

   VkSwapchainCreateInfoKHR createInfo = {};
   createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface                  = vkSurface;

   createInfo.minImageCount    = m_imageCount;
   createInfo.imageFormat      = m_surfaceFormat->format;
   createInfo.imageColorSpace  = m_surfaceFormat->colorSpace;
   createInfo.imageExtent      = *m_extent;
   createInfo.imageArrayLayers = 1;
   createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

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

void Swapchain::_createRenderPasses()
{
   // Initializing main render passes
   CYD::Attachment colorAttachment;
   colorAttachment.type          = CYD::AttachmentType::COLOR_PRESENTATION;
   colorAttachment.format        = TypeConversions::vkToCydFormat( m_surfaceFormat->format );
   colorAttachment.loadOp        = CYD::LoadOp::CLEAR;
   colorAttachment.storeOp       = CYD::StoreOp::STORE;
   colorAttachment.clear.color   = { 0.0f, 0.0f, 0.0f, 1.0f };
   colorAttachment.initialAccess = CYD::Access::UNDEFINED;
   colorAttachment.nextAccess    = CYD::Access::PRESENT;

   CYD::Attachment depthAttachment;
   depthAttachment.type                       = CYD::AttachmentType::DEPTH_STENCIL;
   depthAttachment.format                     = CYD::PixelFormat::D32_SFLOAT;
   depthAttachment.loadOp                     = CYD::LoadOp::CLEAR;
   depthAttachment.storeOp                    = CYD::StoreOp::STORE;
   depthAttachment.clear.depthStencil.depth   = 0.0f;
   depthAttachment.clear.depthStencil.stencil = 0;
   depthAttachment.initialAccess              = CYD::Access::UNDEFINED;
   depthAttachment.nextAccess                 = CYD::Access::DEPTH_STENCIL_ATTACHMENT_WRITE;

   CYD::RenderPassInfo& clearPass = m_renderPasses[0];
   clearPass.attachments.push_back( colorAttachment );
   clearPass.attachments.push_back( depthAttachment );

   colorAttachment.loadOp        = CYD::LoadOp::LOAD;
   colorAttachment.initialAccess = CYD::Access::PRESENT;

   depthAttachment.loadOp        = CYD::LoadOp::LOAD;
   depthAttachment.initialAccess = CYD::Access::DEPTH_STENCIL_ATTACHMENT_WRITE;

   CYD::RenderPassInfo& loadPass = m_renderPasses[1];
   loadPass.attachments.push_back( colorAttachment );
   loadPass.attachments.push_back( depthAttachment );

   m_vkRenderPasses[0] = m_device.getRenderPassCache().findOrCreate( clearPass );
   m_vkRenderPasses[1] = m_device.getRenderPassCache().findOrCreate( loadPass );

   m_colorImageAccess.resize( m_imageCount );
   m_depthImageAccess.resize( m_imageCount );
   for( uint32_t i = 0; i < m_imageCount; ++i )
   {
      m_colorImageAccess[i] = CYD::Access::UNDEFINED;
      m_depthImageAccess[i] = CYD::Access::UNDEFINED;
   }
}

void Swapchain::_createFramebuffers()
{
   VkResult result;

   vkGetSwapchainImagesKHR( m_device.getVKDevice(), m_vkSwapchain, &m_imageCount, nullptr );
   m_colorImages.resize( m_imageCount );
   m_colorImageViews.resize( m_imageCount );
   m_depthImages.resize( m_imageCount );
   m_depthImageViews.resize( m_imageCount );
   m_depthImagesMemory.resize( m_imageCount );
   m_vkFramebuffers.resize( m_imageCount );

   vkGetSwapchainImagesKHR(
       m_device.getVKDevice(), m_vkSwapchain, &m_imageCount, m_colorImages.data() );

   for( size_t i = 0; i < m_imageCount; ++i )
   {
      VkImageCreateInfo imageInfo = {};
      imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType         = VK_IMAGE_TYPE_2D;
      imageInfo.extent.width      = m_extent->width;
      imageInfo.extent.height     = m_extent->height;
      imageInfo.extent.depth      = 1;
      imageInfo.mipLevels         = 1;
      imageInfo.arrayLayers       = 1;
      imageInfo.format            = VK_FORMAT_D32_SFLOAT;
      imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
      imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage   = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      result = vkCreateImage( m_device.getVKDevice(), &imageInfo, nullptr, &m_depthImages[i] );
      CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not create depth image" );

      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements( m_device.getVKDevice(), m_depthImages[i], &memRequirements );

      VkMemoryAllocateInfo allocInfo = {};
      allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize       = memRequirements.size;
      allocInfo.memoryTypeIndex      = m_device.findMemoryType(
          memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

      result =
          vkAllocateMemory( m_device.getVKDevice(), &allocInfo, nullptr, &m_depthImagesMemory[i] );
      CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not allocate depth image memory" );

      vkBindImageMemory( m_device.getVKDevice(), m_depthImages[i], m_depthImagesMemory[i], 0 );

      VkImageViewCreateInfo colorViewInfo           = {};
      colorViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      colorViewInfo.image                           = m_colorImages[i];
      colorViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      colorViewInfo.format                          = m_surfaceFormat->format;
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

      VkImageViewCreateInfo depthViewInfo           = {};
      depthViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      depthViewInfo.image                           = m_depthImages[i];
      depthViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      depthViewInfo.format                          = VK_FORMAT_D32_SFLOAT;
      depthViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
      depthViewInfo.subresourceRange.baseMipLevel   = 0;
      depthViewInfo.subresourceRange.levelCount     = 1;
      depthViewInfo.subresourceRange.baseArrayLayer = 0;
      depthViewInfo.subresourceRange.layerCount     = 1;

      result = vkCreateImageView(
          m_device.getVKDevice(), &depthViewInfo, nullptr, &m_depthImageViews[i] );
      CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not create depth image view" );

      std::vector<VkImageView> vkImageViews;
      vkImageViews.push_back( m_colorImageViews[i] );
      vkImageViews.push_back( m_depthImageViews[i] );

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass              = m_vkRenderPasses[1];  // Load
      framebufferInfo.attachmentCount         = static_cast<uint32_t>( vkImageViews.size() );
      framebufferInfo.pAttachments            = vkImageViews.data();
      framebufferInfo.width                   = m_extent->width;
      framebufferInfo.height                  = m_extent->height;
      framebufferInfo.layers                  = 1;

      VkResult result = vkCreateFramebuffer(
          m_device.getVKDevice(), &framebufferInfo, nullptr, &m_vkFramebuffers[i] );
      CYD_ASSERT( result == VK_SUCCESS && "Swapchain: Could not create framebuffer" );
   }
}

void Swapchain::_createSyncObjects()
{
   m_availableSems.resize( MAX_FRAMES_IN_FLIGHT );
   m_renderDoneSems.resize( MAX_FRAMES_IN_FLIGHT );

   VkSemaphoreCreateInfo semaphoreInfo = {};
   semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
   {
      if( vkCreateSemaphore(
              m_device.getVKDevice(), &semaphoreInfo, nullptr, &m_availableSems[i] ) !=
              VK_SUCCESS ||
          vkCreateSemaphore(
              m_device.getVKDevice(), &semaphoreInfo, nullptr, &m_renderDoneSems[i] ) !=
              VK_SUCCESS )
      {
         CYD_ASSERT( !"Swapchain: Could not create sync objects" );
      }
   }
}

void Swapchain::transitionColorImage( CommandBuffer* cmdBuffer, CYD::Access nextAccess )
{
   Synchronization::ImageMemory(
       cmdBuffer->getVKBuffer(),
       getColorVKImage(),
       1,
       m_surfaceFormat->format,
       getColorVKImageAccess(),
       nextAccess );

   m_colorImageAccess[m_currentFrame] = nextAccess;
}

void Swapchain::transitionDepthImage( CommandBuffer* cmdBuffer, CYD::Access nextAccess )
{
   Synchronization::ImageMemory(
       cmdBuffer->getVKBuffer(),
       getDepthVKImage(),
       1,
       VK_FORMAT_D32_SFLOAT,
       getDepthVKImageAccess(),
       nextAccess );

   m_depthImageAccess[m_currentFrame] = nextAccess;
}

void Swapchain::acquireImage()
{
   vkAcquireNextImageKHR(
       m_device.getVKDevice(),
       m_vkSwapchain,
       UINT64_MAX,
       m_availableSems[m_currentFrame],
       VK_NULL_HANDLE,
       &m_imageIndex );

   m_ready = true;
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

Swapchain::~Swapchain()
{
   for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
   {
      vkDestroySemaphore( m_device.getVKDevice(), m_renderDoneSems[i], nullptr );
      vkDestroySemaphore( m_device.getVKDevice(), m_availableSems[i], nullptr );
   }

   for( uint32_t i = 0; i < m_imageCount; ++i )
   {
      vkDestroyFramebuffer( m_device.getVKDevice(), m_vkFramebuffers[i], nullptr );
      // The color VkImages are managed by the VkSwapchainKHR object, only need to free the rest
      vkDestroyImageView( m_device.getVKDevice(), m_colorImageViews[i], nullptr );
      vkDestroyImage( m_device.getVKDevice(), m_depthImages[i], nullptr );
      vkDestroyImageView( m_device.getVKDevice(), m_depthImageViews[i], nullptr );
      vkFreeMemory( m_device.getVKDevice(), m_depthImagesMemory[i], nullptr );
   }

   vkDestroySwapchainKHR( m_device.getVKDevice(), m_vkSwapchain, nullptr );
}
}
