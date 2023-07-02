#pragma once

#include <Common/Include.h>
#include <Graphics/GraphicsTypes.h>

FWDHANDLE( VkCommandBuffer );

namespace vk
{
class Texture;
class CommandBuffer;
}

namespace vk::Barriers
{
void ImageMemory( VkCommandBuffer cmdBuffer, Texture* texture, CYD::ImageLayout targetLayout );
void Pipeline(
    VkCommandBuffer cmdBuffer,
    CYD::PipelineStageFlag sourceStage,
    CYD::PipelineStageFlag destStage );
}
