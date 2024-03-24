#pragma once

#include <Graphics/GraphicsTypes.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/Vulkan.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace vk::TypeConversions
{
VkIndexType cydToVkIndexType( CYD::IndexType type );
VkFormat cydToVkFormat( CYD::PixelFormat format );
CYD::PixelFormat vkToCydFormat( VkFormat format );
VkColorSpaceKHR cydToVkSpace( CYD::ColorSpace space );
VkAttachmentLoadOp cydToVkOp( CYD::LoadOp op );
VkAttachmentStoreOp cydToVkOp( CYD::StoreOp op );
VkPrimitiveTopology cydToVkDrawPrim( CYD::DrawPrimitive prim );
VkPolygonMode cydToVkPolyMode( CYD::PolygonMode polyMode );
VkCullModeFlagBits cydToVkCullMode( CYD::CullMode cullMode );
uint32_t cydToVkPipelineStages( CYD::PipelineStageFlag stages );
uint32_t cydToVkShaderStages( CYD::PipelineStageFlag stages );
VkDescriptorType cydToVkDescriptorType( CYD::ShaderResourceType type );
VkFilter cydToVkFilter( CYD::Filter filter );
VkBorderColor cydToVkBorderColor( CYD::BorderColor borderColor );
VkSamplerAddressMode cydToVkAddressMode( CYD::AddressMode mode );
VkImageLayout cydToVkImageLayout( CYD::ImageLayout layout );
VkCompareOp cydToVkCompareOp( CYD::CompareOperator op );
uint32_t getAspectMask( CYD::PixelFormat format );
}
