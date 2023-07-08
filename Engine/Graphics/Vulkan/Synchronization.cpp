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
      case CYD::Access::VERTEX_SHADER_READ:
         return THSVS_ACCESS_VERTEX_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER;
      case CYD::Access::FRAGMENT_SHADER_READ:
         return THSVS_ACCESS_FRAGMENT_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER;
      case CYD::Access::COLOR_ATTACHMENT_READ:
         return THSVS_ACCESS_COLOR_ATTACHMENT_READ;
      case CYD::Access::DEPTH_STENCIL_ATTACHMENT_READ:
         return THSVS_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ;
      case CYD::Access::COMPUTE_SHADER_READ:
         return THSVS_ACCESS_COMPUTE_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER;
      case CYD::Access::TRANSFER_READ:
         return THSVS_ACCESS_TRANSFER_READ;
      case CYD::Access::HOST_READ:
         return THSVS_ACCESS_HOST_READ;
      case CYD::Access::VERTEX_SHADER_WRITE:
         return THSVS_ACCESS_VERTEX_SHADER_WRITE;
      case CYD::Access::FRAGMENT_SHADER_WRITE:
         return THSVS_ACCESS_FRAGMENT_SHADER_WRITE;
      case CYD::Access::COLOR_ATTACHMENT_WRITE:
         return THSVS_ACCESS_COLOR_ATTACHMENT_WRITE;
      case CYD::Access::DEPTH_STENCIL_ATTACHMENT_WRITE:
         return THSVS_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE;
      case CYD::Access::COMPUTE_SHADER_WRITE:
         return THSVS_ACCESS_COMPUTE_SHADER_WRITE;
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

void ImageMemory( CommandBuffer* cmdBuffer, Texture* texture, CYD::Access nextAccess )
{
   if( texture->getPreviousAccess() == nextAccess ) return;

   ThsvsAccessType prevAccessThsvs = cydToThsvsAccessType( texture->getPreviousAccess() );
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
   barrier.subresourceRange.baseMipLevel   = 0;
   barrier.subresourceRange.levelCount     = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount     = texture->getLayers();

   thsvsCmdPipelineBarrier( cmdBuffer->getVKBuffer(), nullptr, 0, nullptr, 1, &barrier );

   texture->setPreviousAccess( nextAccess );
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

void Pipeline(
    VkCommandBuffer cmdBuffer,
    CYD::PipelineStageFlag sourceStage,
    CYD::PipelineStageFlag destStage )
{
   vkCmdPipelineBarrier(
       cmdBuffer,
       vk::TypeConversions::cydToVkPipelineStages( sourceStage ),
       vk::TypeConversions::cydToVkPipelineStages( destStage ),
       0,
       0,
       nullptr,
       0,
       nullptr,
       0,
       nullptr );
}
}
