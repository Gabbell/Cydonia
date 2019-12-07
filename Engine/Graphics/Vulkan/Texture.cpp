#include <Graphics/Vulkan/Texture.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/SamplerStash.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk
{
void Texture::seize( const Device& device, const cyd::TextureDescription& desc )
{
   _device = &device;
   _size   = desc.size;
   _width  = desc.width;
   _height = desc.height;
   _type   = desc.type;
   _format = desc.format;
   _usage  = desc.usage;
   _layout = cyd::ImageLayout::UNDEFINED;
   _inUse  = true;

   _createImage();
   _allocateMemory();
   _createImageView();
}

void Texture::release()
{
   if( _device )
   {
      _size   = 0;
      _width  = 0;
      _height = 0;
      _type   = cyd::ImageType::TEXTURE_2D;
      _format = cyd::PixelFormat::BGRA8_UNORM;
      _usage  = 0;
      _layout = cyd::ImageLayout::UNDEFINED;
      _inUse  = false;

      vkDestroyImageView( _device->getVKDevice(), _vkImageView, nullptr );
      vkDestroyImage( _device->getVKDevice(), _vkImage, nullptr );
      vkFreeMemory( _device->getVKDevice(), _vkMemory, nullptr );

      _device      = nullptr;
      _vkImageView = nullptr;
      _vkImage     = nullptr;
      _vkMemory    = nullptr;
   }
}

void Texture::_createImage()
{
   // Creating image
   VkImageCreateInfo imageInfo = {};
   imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

   switch( _type )
   {
      case cyd::ImageType::TEXTURE_1D:
         imageInfo.imageType = VK_IMAGE_TYPE_1D;
         break;
      case cyd::ImageType::TEXTURE_2D:
         imageInfo.imageType = VK_IMAGE_TYPE_2D;
         break;
      case cyd::ImageType::TEXTURE_3D:
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

   if( _usage & cyd::ImageUsage::TRANSFER_SRC )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
   }
   if( _usage & cyd::ImageUsage::TRANSFER_DST )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   }
   if( _usage & cyd::ImageUsage::SAMPLED )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
   }
   if( _usage & cyd::ImageUsage::STORAGE )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
   }
   if( _usage & cyd::ImageUsage::COLOR )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   if( _usage & cyd::ImageUsage::DEPTH_STENCIL )
   {
      imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   }

   imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VkResult result = vkCreateImage( _device->getVKDevice(), &imageInfo, nullptr, &_vkImage );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not create image" );
}

void Texture::_allocateMemory()
{
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements( _device->getVKDevice(), _vkImage, &memRequirements );

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize       = memRequirements.size;
   allocInfo.memoryTypeIndex      = _device->findMemoryType(
       memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

   VkResult result = vkAllocateMemory( _device->getVKDevice(), &allocInfo, nullptr, &_vkMemory );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not allocate memory" );

   vkBindImageMemory( _device->getVKDevice(), _vkImage, _vkMemory, 0 );
}

void Texture::_createImageView()
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

   VkResult result = vkCreateImageView( _device->getVKDevice(), &viewInfo, nullptr, &_vkImageView );
   CYDASSERT( result == VK_SUCCESS && "Texture: Could not create image view" );
}

void Texture::updateDescriptorSet( const cyd::ShaderObjectInfo& info, VkDescriptorSet descSet )
{
   _vkDescSet = descSet;

   const VkSampler vkSampler = _device->getSamplerStash().findOrCreate( {} );

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

   vkUpdateDescriptorSets( _device->getVKDevice(), 1, &descriptorWrite, 0, nullptr );
}
}
