#pragma once

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Forwards
// ================================================================================================
enum VkIndexType : int;
enum VkFormat : int;
enum VkColorSpaceKHR : int;
enum VkCompareOp : int;
enum VkAttachmentLoadOp : int;
enum VkAttachmentStoreOp : int;
enum VkPrimitiveTopology : int;
enum VkPolygonMode : int;
enum VkImageLayout : int;
enum VkDescriptorType : int;
enum VkFilter : int;
enum VkBorderColor : int;
enum VkSamplerAddressMode : int;

// ================================================================================================
// Definition
// ================================================================================================
namespace vk::TypeConversions
{
VkIndexType cydToVkIndexType( CYD::IndexType type );
VkFormat cydToVkFormat( CYD::PixelFormat format );
VkColorSpaceKHR cydToVkSpace( CYD::ColorSpace space );
VkAttachmentLoadOp cydToVkOp( CYD::LoadOp op );
VkAttachmentStoreOp cydToVkOp( CYD::StoreOp op );
VkPrimitiveTopology cydToVkDrawPrim( CYD::DrawPrimitive prim );
VkPolygonMode cydToVkPolyMode( CYD::PolygonMode polyMode );
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
