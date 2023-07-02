#include <Graphics/Vulkan/BarriersHelper.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk::Barriers
{
void ImageMemory( VkCommandBuffer cmdBuffer, Texture* texture, CYD::ImageLayout targetLayout )
{
   CYD_ASSERT( texture && "BarriersHelper: No texture passed to make barrier" );

   const CYD::ImageLayout initialLayout      = texture->getLayout();
   const CYD::PipelineStageFlag targetStages = texture->getStages();

   // If the layout is the same, don't even insert the barrier. The only point of this barrier is to
   // transition layouts currently
   if( initialLayout == targetLayout ) return;

   // Transition image layout to transfer destination optimal
   VkImageMemoryBarrier barrier = {};
   barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout            = TypeConversions::cydToVkImageLayout( initialLayout );
   barrier.newLayout            = TypeConversions::cydToVkImageLayout( targetLayout );
   barrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
   barrier.image                = texture->getVKImage();

   barrier.subresourceRange.aspectMask =
       TypeConversions::getAspectMask( texture->getPixelFormat() );
   barrier.subresourceRange.baseMipLevel   = 0;
   barrier.subresourceRange.levelCount     = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount     = texture->getLayers();

   // These paremeters are dynamic based on the current command buffer and texture state
   VkPipelineStageFlags srcStageMask = 0;
   VkPipelineStageFlags dstStageMask = 0;

   switch( initialLayout )
   {
      case CYD::ImageLayout::UNKNOWN:
      case CYD::ImageLayout::GENERAL:
         barrier.srcAccessMask |= 0;
         srcStageMask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
         break;
      case CYD::ImageLayout::COLOR_ATTACHMENT:
         barrier.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
         srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
         break;
      case CYD::ImageLayout::DEPTH_STENCIL_ATTACHMENT:
         barrier.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
         srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
         break;
      case CYD::ImageLayout::TRANSFER_DST:
         barrier.srcAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
         srcStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
         break;
      default:
         CYD_ASSERT( !"BarriersHelper: Could not determine source access mask based on current image layout for barrier" );
   }

   switch( targetLayout )
   {
      case CYD::ImageLayout::TRANSFER_DST:
         barrier.dstAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
         dstStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
         break;
      case CYD::ImageLayout::SHADER_READ:
         barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
         if( targetStages & CYD::PipelineStage::FRAGMENT_STAGE )
         {
            dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
         }
         if( targetStages & CYD::PipelineStage::COMPUTE_STAGE )
         {
            dstStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
         }
         break;
      case CYD::ImageLayout::DEPTH_STENCIL_READ:
         barrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
         if( targetStages & CYD::PipelineStage::FRAGMENT_STAGE )
         {
            dstStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
         }
         if( targetStages & CYD::PipelineStage::COMPUTE_STAGE )
         {
            dstStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
         }
         break;
      case CYD::ImageLayout::GENERAL:
         barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
         if( targetStages & CYD::PipelineStage::FRAGMENT_STAGE )
         {
            dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
         }
         if( targetStages & CYD::PipelineStage::COMPUTE_STAGE )
         {
            dstStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
         }
         break;
      default:
         CYD_ASSERT( !"BarriersHelper: Could not determine destination access mask based on target image layout for barrier" );
   }

   vkCmdPipelineBarrier(
       cmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier );

   // Updating layout
   texture->setLayout( targetLayout );
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
