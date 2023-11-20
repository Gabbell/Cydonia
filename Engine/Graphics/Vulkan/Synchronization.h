#pragma once

#include <Common/Include.h>
#include <Graphics/GraphicsTypes.h>
#include <Graphics/Vulkan.h>

FWDHANDLE( VkCommandBuffer );
FWDHANDLE( VkImage );

namespace vk
{
class Texture;
class CommandBuffer;
}

namespace vk::Synchronization
{
VkImageLayout GetLayoutFromAccess( CYD::Access prevAccess );

//  Can only transition one mip level at a time for now
void ImageMemory(
    const CommandBuffer* cmdBuffer,
    Texture* texture,
    CYD::Access nextAccess,
    uint32_t mipLevel = 0 );

void ImageMemory(
    VkCommandBuffer cmdBuffer,
    VkImage image,
    uint32_t layers,
    VkFormat format,
    CYD::Access prevAccess,
    CYD::Access nextAccess );

void GlobalMemory( VkCommandBuffer cmdBuffer, CYD::Access prevAccess, CYD::Access nextAccess );
}
