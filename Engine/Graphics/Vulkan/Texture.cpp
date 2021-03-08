#include <Graphics/Vulkan/Texture.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk
{
Texture::Texture() { m_useCount = std::make_unique<std::atomic<uint32_t>>( 0 ); }

void Texture::acquire( const Device& device, const CYD::TextureDescription& desc )
{
   m_pDevice = &device;
   m_size    = desc.size;
   m_width   = desc.width;
   m_height  = desc.height;
   m_layers  = desc.layers;
   m_type    = desc.type;
   m_format  = desc.format;
   m_usage   = desc.usage;
   m_stages  = desc.stages;

   _createImage();
   _allocateMemory();
   _createImageView();

   incUse();
}

void Texture::release()
{
   if( m_pDevice )
   {
      CYDASSERT( m_useCount->load() == 0 && "Texture: released a still used texture" );

      vkDestroyImageView( m_pDevice->getVKDevice(), m_vkImageView, nullptr );
      vkDestroyImage( m_pDevice->getVKDevice(), m_vkImage, nullptr );
      vkFreeMemory( m_pDevice->getVKDevice(), m_vkMemory, nullptr );

      m_size   = 0;
      m_width  = 0;
      m_height = 0;
      m_layers = 1;
      m_type   = CYD::ImageType::TEXTURE_2D;
      m_format = CYD::PixelFormat::RGBA8_SRGB;
      m_usage  = 0;
      m_stages = 0;

      m_pDevice     = nullptr;
      m_vkImageView = nullptr;
      m_vkImage     = nullptr;
      m_vkMemory    = nullptr;
   }
}

void Texture::_createImage()
{
   // Creating image
   VkImageCreateInfo imageInfo = {};
   imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

   switch( m_type )
   {
      case CYD::ImageType::TEXTURE_1D:
         imageInfo.imageType = VK_IMAGE_TYPE_1D;
         break;
      case CYD::ImageType::TEXTURE_2D:
         imageInfo.imageType = VK_IMAGE_TYPE_2D;
         break;
      case CYD::ImageType::TEXTURE_3D:
         imageInfo.imageType = VK_IMAGE_TYPE_3D;
         break;
   }

   // TODO A 6-layer image is always "promoted" to a cube map
   if( m_layers == 6 )
   {
      imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
   }

   imageInfo.extent.width  = m_width;
   imageInfo.extent.height = m_height;
   imageInfo.extent.depth  = 1;
   imageInfo.mipLevels     = 1;
   imageInfo.arrayLayers   = m_layers;
   imageInfo.format        = TypeConversions::cydToVkFormat( m_format );
   imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
   imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

   if( m_usage & CYD::ImageUsage::TRANSFER_SRC )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
   }
   if( m_usage & CYD::ImageUsage::TRANSFER_DST )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   }
   if( m_usage & CYD::ImageUsage::SAMPLED )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
   }
   if( m_usage & CYD::ImageUsage::STORAGE )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
   }
   if( m_usage & CYD::ImageUsage::COLOR )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   if( m_usage & CYD::ImageUsage::DEPTH_STENCIL )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   }

   imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VkResult result = vkCreateImage( m_pDevice->getVKDevice(), &imageInfo, nullptr, &m_vkImage );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not create image" );
}

void Texture::_allocateMemory()
{
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements( m_pDevice->getVKDevice(), m_vkImage, &memRequirements );

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize       = memRequirements.size;
   allocInfo.memoryTypeIndex      = m_pDevice->findMemoryType(
       memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

   VkResult result = vkAllocateMemory( m_pDevice->getVKDevice(), &allocInfo, nullptr, &m_vkMemory );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not allocate memory" );

   vkBindImageMemory( m_pDevice->getVKDevice(), m_vkImage, m_vkMemory, 0 );
}

void Texture::_createImageView()
{
   VkImageViewCreateInfo viewInfo = {};
   viewInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewInfo.image                 = m_vkImage;

   if( m_width > 0 )
   {
      if( m_height > 0 )
      {
         if( m_layers == 6 )
         {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
         }
         else if( m_layers > 1 )
         {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
         }
         else
         {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
         }
      }
      else
      {
         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
      }
   }
   else
   {
      CYDASSERT( !"Texture: Could not determine image view type" );
   }

   viewInfo.format                          = TypeConversions::cydToVkFormat( m_format );
   viewInfo.subresourceRange.aspectMask     = TypeConversions::getAspectMask( m_format );
   viewInfo.subresourceRange.baseMipLevel   = 0;
   viewInfo.subresourceRange.levelCount     = 1;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount     = m_layers;

   VkResult result =
       vkCreateImageView( m_pDevice->getVKDevice(), &viewInfo, nullptr, &m_vkImageView );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not create image view" );
}

Texture::~Texture() { release(); }
}
