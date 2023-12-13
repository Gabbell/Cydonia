#include <Graphics/Vulkan/Synchronization.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/TypeConversions.h>

#define THSVS_SIMPLER_VULKAN_SYNCHRONIZATION_IMPLEMENTATION
#include <simple_vulkan_synchronization/thsvs_simpler_vulkan_synchronization.h>

namespace vk::Synchronization
{
static ThsvsAccessType cydToThsvsAccessType( CYD::Access access )
{
   switch( access )
   {
      case CYD::Access::UNDEFINED:
         return THSVS_ACCESS_NONE;
      case CYD::Access::VERTEX_SHADER_READ:  // This access assumes image or uniform texel buffer
         return THSVS_ACCESS_VERTEX_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER;
      case CYD::Access::FRAGMENT_SHADER_READ:
         return THSVS_ACCESS_FRAGMENT_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER;
      case CYD::Access::COMPUTE_SHADER_READ:
         return THSVS_ACCESS_COMPUTE_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER;
      case CYD::Access::ANY_SHADER_READ:
         return THSVS_ACCESS_ANY_SHADER_READ_OTHER;
      case CYD::Access::DEPTH_STENCIL_ATTACHMENT_READ:
         return THSVS_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ;
      case CYD::Access::TRANSFER_READ:
         return THSVS_ACCESS_TRANSFER_READ;
      case CYD::Access::HOST_READ:
         return THSVS_ACCESS_HOST_READ;
      case CYD::Access::VERTEX_SHADER_WRITE:
         return THSVS_ACCESS_VERTEX_SHADER_WRITE;
      case CYD::Access::FRAGMENT_SHADER_WRITE:
         return THSVS_ACCESS_FRAGMENT_SHADER_WRITE;
      case CYD::Access::COMPUTE_SHADER_WRITE:
         return THSVS_ACCESS_COMPUTE_SHADER_WRITE;
      case CYD::Access::ANY_SHADER_WRITE:
         return THSVS_ACCESS_ANY_SHADER_WRITE;
      case CYD::Access::COLOR_ATTACHMENT_WRITE:
         return THSVS_ACCESS_COLOR_ATTACHMENT_WRITE;
      case CYD::Access::DEPTH_STENCIL_ATTACHMENT_WRITE:
         return THSVS_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE;
      case CYD::Access::TRANSFER_WRITE:
         return THSVS_ACCESS_TRANSFER_WRITE;
      case CYD::Access::HOST_WRITE:
         return THSVS_ACCESS_HOST_WRITE;
      case CYD::Access::PRESENT:
         return THSVS_ACCESS_PRESENT;
      case CYD::Access::GENERAL:
         return THSVS_ACCESS_GENERAL;
      default:
         CYD_ASSERT( !"Synchronization: Could not determine access type" );
   }

   return THSVS_ACCESS_NONE;
}

VkImageLayout GetLayoutFromAccess( CYD::Access access )
{
   return ThsvsAccessMap[cydToThsvsAccessType( access )].imageLayout;
}

void ImageMemory(
    const CommandBuffer* cmdBuffer,
    Texture* texture,
    CYD::Access nextAccess,
    uint32_t mipLevel,
    uint32_t layer )
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

   if( texture->getPreviousAccess( layer, mipLevel ) == nextAccess ) return;

   ThsvsAccessType prevAccessThsvs =
       cydToThsvsAccessType( texture->getPreviousAccess( layer, mipLevel ) );
   ThsvsAccessType nextAccessThsvs = cydToThsvsAccessType( nextAccess );

   ThsvsImageBarrier barrier;
   barrier.prevAccessCount     = 1;
   barrier.pPrevAccesses       = &prevAccessThsvs;
   barrier.nextAccessCount     = 1;
   barrier.pNextAccesses       = &nextAccessThsvs;
   barrier.prevLayout          = THSVS_IMAGE_LAYOUT_OPTIMAL;
   barrier.nextLayout          = THSVS_IMAGE_LAYOUT_OPTIMAL;
   barrier.discardContents     = false;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image               = texture->getVKImage();

   barrier.subresourceRange.aspectMask =
       TypeConversions::getAspectMask( texture->getPixelFormat() );

   barrier.subresourceRange.baseMipLevel   = allMipLevels ? 0 : mipLevel;
   barrier.subresourceRange.levelCount     = allMipLevels ? VK_REMAINING_ARRAY_LAYERS : 1;
   barrier.subresourceRange.baseArrayLayer = allArrayLayers ? 0 : layer;
   barrier.subresourceRange.layerCount     = allArrayLayers ? VK_REMAINING_ARRAY_LAYERS : 1;

   thsvsCmdPipelineBarrier( cmdBuffer->getVKCmdBuffer(), nullptr, 0, nullptr, 1, &barrier );

   texture->setPreviousAccess( nextAccess, layer, mipLevel );
}

void ImageMemory(
    VkCommandBuffer cmdBuffer,
    VkImage image,
    uint32_t layers,
    VkFormat format,
    CYD::Access prevAccess,
    CYD::Access nextAccess )
{
   if( prevAccess == nextAccess ) return;

   ThsvsAccessType prevAccessThsvs = cydToThsvsAccessType( prevAccess );
   ThsvsAccessType nextAccessThsvs = cydToThsvsAccessType( nextAccess );

   ThsvsImageBarrier barrier;
   barrier.prevAccessCount     = 1;
   barrier.pPrevAccesses       = &prevAccessThsvs;
   barrier.nextAccessCount     = 1;
   barrier.pNextAccesses       = &nextAccessThsvs;
   barrier.prevLayout          = THSVS_IMAGE_LAYOUT_OPTIMAL;
   barrier.nextLayout          = THSVS_IMAGE_LAYOUT_OPTIMAL;
   barrier.discardContents     = false;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image               = image;

   barrier.subresourceRange.aspectMask =
       TypeConversions::getAspectMask( TypeConversions::vkToCydFormat( format ) );
   barrier.subresourceRange.baseMipLevel   = 0;
   barrier.subresourceRange.levelCount     = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount     = layers;

   thsvsCmdPipelineBarrier( cmdBuffer, nullptr, 0, nullptr, 1, &barrier );
}

void SyncQueue( VkCommandBuffer cmdBuffer, CYD::PipelineType type )
{
   ThsvsGlobalBarrier barrier;

   if( type == CYD::PipelineType::GRAPHICS )
   {
      ThsvsAccessType prevAccess[3] = {
          THSVS_ACCESS_COLOR_ATTACHMENT_WRITE,
          THSVS_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE,
          THSVS_ACCESS_FRAGMENT_SHADER_WRITE };

      ThsvsAccessType nextAccess[3] = {
          THSVS_ACCESS_COLOR_ATTACHMENT_READ,
          THSVS_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ,
          THSVS_ACCESS_FRAGMENT_SHADER_READ_OTHER };

      barrier.prevAccessCount = 3;
      barrier.pPrevAccesses   = prevAccess;
      barrier.nextAccessCount = 3;
      barrier.pNextAccesses   = nextAccess;

      thsvsCmdPipelineBarrier( cmdBuffer, &barrier, 0, nullptr, 0, nullptr );
   }
   else if( type == CYD::PipelineType::COMPUTE )
   {
      ThsvsAccessType prevAccess[1] = { THSVS_ACCESS_COMPUTE_SHADER_WRITE };

      ThsvsAccessType nextAccess[1] = { THSVS_ACCESS_COMPUTE_SHADER_READ_OTHER };

      barrier.prevAccessCount = 1;
      barrier.pPrevAccesses   = prevAccess;
      barrier.nextAccessCount = 1;
      barrier.pNextAccesses   = nextAccess;

      thsvsCmdPipelineBarrier( cmdBuffer, &barrier, 0, nullptr, 0, nullptr );
   }
}
}
