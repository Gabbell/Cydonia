#pragma once

#include <Common/Include.h>
#include <Graphics/GraphicsTypes.h>

FWDHANDLE( VkCommandBuffer );
FWDHANDLE( VkImage );

namespace vk
{
class Texture;
class CommandBuffer;
}

enum VkFormat : int;
enum VkImageLayout : int;

namespace vk::Synchronization
{
VkImageLayout GetLayoutFromAccess( CYD::Access prevAccess );

void ImageMemory( CommandBuffer* cmdBuffer, Texture* texture, CYD::Access nextAccess );

void ImageMemory(
    VkCommandBuffer cmdBuffer,
    VkImage image,
    uint32_t layers,
    VkFormat format,
    CYD::Access prevAccess,
    CYD::Access nextAccess);

void Pipeline(
    VkCommandBuffer cmdBuffer,
    CYD::PipelineStageFlag sourceStage,
    CYD::PipelineStageFlag destStage );
}
