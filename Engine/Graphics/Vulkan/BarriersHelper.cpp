#include <Graphics/Vulkan/BarriersHelper.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Pipelines.h>

#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk::Barriers
{
void ImageMemory( const CommandBuffer* cmdBuffer, Texture* texture, CYD::ImageLayout targetLayout )
{
   CYDASSERT( texture && "BarriersHelper: No texture passed to make barrier" );

   const CYD::ImageLayout initialLayout    = texture->getLayout();
   const CYD::ShaderStageFlag targetStages = texture->getStages();

   // Transition image layout to transfer destination optimal
   VkImageMemoryBarrier barrier            = {};
   barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout                       = TypeConversions::cydToVkImageLayout( initialLayout );
   barrier.newLayout                       = TypeConversions::cydToVkImageLayout( targetLayout );
   barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
   barrier.image                           = texture->getVKImage();
   barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.baseMipLevel   = 0;
   barrier.subresourceRange.levelCount     = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount     = texture->getLayers();

   // These paremeters are dynamic based on the current command buffer and texture state
   VkPipelineStageFlags srcPipelineStage = 0;
   VkPipelineStageFlags dstPipelineStage = 0;

   switch( initialLayout )
   {
      case CYD::ImageLayout::UNKNOWN:
      case CYD::ImageLayout::GENERAL:
         barrier.srcAccessMask |= 0;
         srcPipelineStage |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
         break;
      case CYD::ImageLayout::TRANSFER_DST:
         barrier.srcAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
         srcPipelineStage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
         break;
      default:
         CYDASSERT( !"BarriersHelper: Could not determine source access mask based on current image layout for barrier" );
   }

   switch( targetLayout )
   {
      case CYD::ImageLayout::TRANSFER_DST:
         barrier.dstAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
         break;
      case CYD::ImageLayout::SHADER_READ:
         barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
         break;
      case CYD::ImageLayout::GENERAL:
         barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
         break;
      default:
         CYDASSERT( !"BarriersHelper: Could not determine destination access mask based on target image layout for barrier" );
   }

   if( targetLayout == CYD::ImageLayout::TRANSFER_DST )
   {
      dstPipelineStage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
   }
   else if(
       ( targetLayout == CYD::ImageLayout::SHADER_READ ) ||
       ( targetLayout == CYD::ImageLayout::GENERAL ) )
   {
      if( targetStages & CYD::ShaderStage::FRAGMENT_STAGE )
      {
         dstPipelineStage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      }
      if( targetStages & CYD::ShaderStage::COMPUTE_STAGE )
      {
         dstPipelineStage |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
      }
   }

   vkCmdPipelineBarrier(
       cmdBuffer->getVKBuffer(),
       srcPipelineStage,
       dstPipelineStage,
       0,
       0,
       nullptr,
       0,
       nullptr,
       1,
       &barrier );

   // Updating layout
   texture->setLayout( targetLayout );
}
}
