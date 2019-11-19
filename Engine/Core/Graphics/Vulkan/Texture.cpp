#include <Core/Graphics/Vulkan/Texture.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/SamplerStash.h>
#include <Core/Graphics/Vulkan/TypeConversions.h>

cyd::Texture::Texture( const Device& device, const TextureDescription& desc )
    : _device( device ),
      _size( desc.size ),
      _width( desc.width ),
      _height( desc.height ),
      _type( desc.type ),
      _format( desc.format ),
      _usage( desc.usage ),
      _layout( ImageLayout::UNDEFINED )
{
   _createImage();
   _allocateMemory();
   _createImageView();
}

void cyd::Texture::_createImage()
{
   // Creating image
   VkImageCreateInfo imageInfo = {};
   imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

   switch( _type )
   {
      case ImageType::TEXTURE_1D:
         imageInfo.imageType = VK_IMAGE_TYPE_1D;
         break;
      case ImageType::TEXTURE_2D:
         imageInfo.imageType = VK_IMAGE_TYPE_2D;
         break;
      case ImageType::TEXTURE_3D:
         imageInfo.imageType = VK_IMAGE_TYPE_3D;
         break;
   }

   imageInfo.extent.width  = _width;
   imageInfo.extent.height = _height;
   imageInfo.extent.depth  = 1;
   imageInfo.mipLevels     = 1;
   imageInfo.arrayLayers   = 1;
   imageInfo.format        = TypeConversions::cydFormatToVkFormat( _format );
   imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
   imageInfo.initialLayout = TypeConversions::cydImageLayoutToVKImageLayout( _layout );

   if( _usage & ImageUsage::TRANSFER_SRC )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
   }
   if( _usage & ImageUsage::TRANSFER_DST )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   }
   if( _usage & ImageUsage::SAMPLED )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
   }
   if( _usage & ImageUsage::STORAGE )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
   }
   if( _usage & ImageUsage::COLOR )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   if( _usage & ImageUsage::DEPTH_STENCIL )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   }

   imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VkResult result = vkCreateImage( _device.getVKDevice(), &imageInfo, nullptr, &_vkImage );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not create image" );
}

void cyd::Texture::_allocateMemory()
{
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements( _device.getVKDevice(), _vkImage, &memRequirements );

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize       = memRequirements.size;
   allocInfo.memoryTypeIndex      = _device.findMemoryType(
       memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

   VkResult result = vkAllocateMemory( _device.getVKDevice(), &allocInfo, nullptr, &_vkMemory );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not allocate memory" );

   vkBindImageMemory( _device.getVKDevice(), _vkImage, _vkMemory, 0 );
}

void cyd::Texture::_createImageView()
{
   VkImageViewCreateInfo viewInfo           = {};
   viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewInfo.image                           = _vkImage;
   viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
   viewInfo.format                          = TypeConversions::cydFormatToVkFormat( _format );
   viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   viewInfo.subresourceRange.baseMipLevel   = 0;
   viewInfo.subresourceRange.levelCount     = 1;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount     = 1;

   VkResult result = vkCreateImageView( _device.getVKDevice(), &viewInfo, nullptr, &_vkImageView );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not create image view" );
}

void cyd::Texture::updateDescriptorSet( const ShaderObjectInfo& info, VkDescriptorSet descSet )
{
   _vkDescSet = descSet;

   const VkSampler vkSampler = _device.getSamplerStash().findOrCreate( {} );

   VkDescriptorImageInfo imageInfo = {};
   // By the point we bind this texture, it should have transferred to this layout
   imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   imageInfo.imageView   = _vkImageView;
   imageInfo.sampler     = vkSampler;

   VkWriteDescriptorSet descriptorWrite = {};
   descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrite.dstSet               = _vkDescSet;
   descriptorWrite.dstBinding           = info.binding;
   descriptorWrite.dstArrayElement      = 0;
   descriptorWrite.descriptorType =
       TypeConversions::cydShaderObjectTypeToVkDescriptorType( info.type );
   descriptorWrite.descriptorCount = 1;
   descriptorWrite.pImageInfo      = &imageInfo;

   vkUpdateDescriptorSets( _device.getVKDevice(), 1, &descriptorWrite, 0, nullptr );
}

cyd::Texture::~Texture()
{
   vkDestroyImageView( _device.getVKDevice(), _vkImageView, nullptr );
   vkDestroyImage( _device.getVKDevice(), _vkImage, nullptr );
   vkFreeMemory( _device.getVKDevice(), _vkMemory, nullptr );
}
