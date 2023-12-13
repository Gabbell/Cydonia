#include <Graphics/Vulkan/Texture.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>
#include <Graphics/Vulkan/Swapchain.h>

namespace vk
{
Texture::Texture()
{
   m_useCount = std::make_unique<std::atomic<uint32_t>>( 0 );
   _clearPreviousAccess();
}

void Texture::acquire( const Device& device, const CYD::TextureDescription& desc )
{
   m_pDevice = &device;
   m_width   = desc.width;
   m_height  = desc.height;
   m_depth   = desc.depth;
   m_type    = desc.type;
   m_format  = desc.format;
   m_usage   = desc.usage;
   m_stages  = desc.stages;
   m_name    = desc.name;

   if( desc.generateMipmaps )
   {
      m_mipLevels =
          static_cast<uint32_t>( std::floor( std::log2( std::max( m_width, m_height ) ) ) ) + 1;
   }
   else
   {
      m_mipLevels = 1;
   }

   _createImage();
   _allocateMemory();

   m_vkImageView = _createImageView();

   incUse();
}

void Texture::release()
{
   if( m_pDevice )
   {
      CYD_ASSERT( m_useCount->load() == 0 && "Texture: released a still used texture" );

      vkDestroyImageView( m_pDevice->getVKDevice(), m_vkImageView, nullptr );

      for( uint32_t mipLevel = 0; mipLevel < m_mipLevels; ++mipLevel )
      {
         for( uint32_t layer = 0; layer < m_depth; ++layer )
         {
            if( m_vkImageViews[mipLevel][layer] != VK_NULL_HANDLE )
            {
               vkDestroyImageView(
                   m_pDevice->getVKDevice(), m_vkImageViews[mipLevel][layer], nullptr );
            }
         }
      }

      vkDestroyImage( m_pDevice->getVKDevice(), m_vkImage, nullptr );
      vkFreeMemory( m_pDevice->getVKDevice(), m_vkMemory, nullptr );

      _clearPreviousAccess();

      m_size        = 0;
      m_width       = 0;
      m_height      = 0;
      m_depth       = 1;
      m_mipLevels   = 1;
      m_type        = CYD::ImageType::TEXTURE_2D;
      m_format      = CYD::PixelFormat::UNKNOWN;
      m_usage       = 0;
      m_stages      = 0;
      m_pDevice     = VK_NULL_HANDLE;
      m_vkImageView = VK_NULL_HANDLE;
      m_vkImage     = VK_NULL_HANDLE;
      m_vkMemory    = VK_NULL_HANDLE;

      memset( m_vkImageViews.data(), 0, sizeof( m_vkImageViews ) );
   }
}

CYD::Access Texture::getPreviousAccess( uint32_t layer, uint32_t mipLevel ) const
{
   CYD_ASSERT( mipLevel < m_mipLevels );
   if( mipLevel < m_mipLevels )
   {
      return m_prevAccesses[mipLevel][layer];
   }

   return CYD::Access::UNDEFINED;
}

void Texture::setPreviousAccess( CYD::Access access, uint32_t layer, uint32_t mipLevel )
{
   CYD_ASSERT( mipLevel < m_mipLevels );
   if( mipLevel < m_mipLevels )
   {
      m_prevAccesses[mipLevel][layer] = access;
   }
}

void Texture::_clearPreviousAccess()
{
   for( uint32_t mipLevel = 0; mipLevel < MAX_MIP_LEVEL; ++mipLevel )
   {
      for( uint32_t layer = 0; layer < MAX_DEPTH_LEVEL; ++layer )
      {
         m_prevAccesses[mipLevel][layer] = CYD::Access::UNDEFINED;
      }
   }
}

void Texture::incUse() { ( *m_useCount )++; }
void Texture::decUse()
{
   if( m_useCount->load() == 0 )
   {
      CYD_ASSERT(
          !"Tried to decrease a resource usage counter to below 0. Probably a scope problem." );
      return;
   }
   ( *m_useCount )--;
}

uint32_t Texture::getLayerCount() const
{
   if( m_type == CYD::ImageType::TEXTURE_2D_ARRAY || m_type == CYD::ImageType::TEXTURE_CUBE ||
       m_type == CYD::ImageType::TEXTURE_CUBE_ARRAY )
   {
      return m_depth;
   }

   return 1;
}

uint32_t Texture::getDepth() const
{
   if( m_type == CYD::ImageType::TEXTURE_3D )
   {
      return m_depth;
   }

   return 1;
}

VkImageView Texture::getIndexedVKImageView( uint32_t layer, uint32_t mipLevel )
{
   // These image views are lazy-initialized because they should only be created if needed
   if( m_vkImageViews[mipLevel][layer] == VK_NULL_HANDLE )
   {
      m_vkImageViews[mipLevel][layer] = _createImageView( layer, mipLevel );
   }

   return m_vkImageViews[mipLevel][layer];
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
      case CYD::ImageType::TEXTURE_2D_ARRAY:
      case CYD::ImageType::TEXTURE_CUBE:
      case CYD::ImageType::TEXTURE_CUBE_ARRAY:
         imageInfo.imageType = VK_IMAGE_TYPE_2D;
         break;
      case CYD::ImageType::TEXTURE_3D:
         imageInfo.imageType = VK_IMAGE_TYPE_3D;
         break;
   }

   // TODO A 6-layer 2D image is always "promoted" to a cube map
   if( m_depth == 6 && m_type == CYD::ImageType::TEXTURE_2D )
   {
      imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
   }

   imageInfo.extent.width  = m_width;
   imageInfo.extent.height = m_height;
   imageInfo.mipLevels     = m_mipLevels;
   imageInfo.format        = TypeConversions::cydToVkFormat( m_format );
   imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
   imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   imageInfo.extent.depth  = getDepth();
   imageInfo.arrayLayers   = getLayerCount();

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
   CYD_ASSERT( result == VK_SUCCESS && "Texture: Could not create image" );
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
   CYD_ASSERT( result == VK_SUCCESS && "Texture: Could not allocate memory" );

   vkBindImageMemory( m_pDevice->getVKDevice(), m_vkImage, m_vkMemory, 0 );
}

VkImageView Texture::_createImageView( uint32_t layer, uint32_t mipLevel )
{
   bool allMipLevels = false;
   if( mipLevel == CYD::ALL_MIP_LEVELS )
   {
      mipLevel     = 0;
      allMipLevels = true;
   }

   bool allArrayLayers = false;
   if( layer == CYD::ALL_ARRAY_LAYERS )
   {
      layer          = 0;
      allArrayLayers = true;
   }

   VkImageViewCreateInfo viewInfo = {};
   viewInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewInfo.image                 = m_vkImage;

   switch( m_type )
   {
      case CYD::ImageType::TEXTURE_1D:
         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
         break;
      case CYD::ImageType::TEXTURE_2D:
         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
         break;
      case CYD::ImageType::TEXTURE_3D:
         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
         break;
      case CYD::ImageType::TEXTURE_2D_ARRAY:
         if( allArrayLayers )
         {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
         }
         else
         {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
         }
         break;
      case CYD::ImageType::TEXTURE_CUBE:
         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
         break;
      case CYD::ImageType::TEXTURE_CUBE_ARRAY:
         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
         break;
      default:
         CYD_ASSERT( !"Texture: Unrecognized image type" );
   }

   viewInfo.format                          = TypeConversions::cydToVkFormat( m_format );
   viewInfo.subresourceRange.aspectMask     = TypeConversions::getAspectMask( m_format );
   viewInfo.subresourceRange.baseMipLevel   = allMipLevels ? 0 : mipLevel;
   viewInfo.subresourceRange.levelCount     = allMipLevels ? VK_REMAINING_MIP_LEVELS : 1;
   viewInfo.subresourceRange.baseArrayLayer = allArrayLayers ? 0 : layer;
   viewInfo.subresourceRange.layerCount     = allArrayLayers ? VK_REMAINING_ARRAY_LAYERS : 1;

   VkImageView vkImageView;

   VkResult result =
       vkCreateImageView( m_pDevice->getVKDevice(), &viewInfo, nullptr, &vkImageView );
   CYD_ASSERT( result == VK_SUCCESS && "Texture: Could not create image view" );

   return vkImageView;
}

Texture::~Texture() { release(); }
}
