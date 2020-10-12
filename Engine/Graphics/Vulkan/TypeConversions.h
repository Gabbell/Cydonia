#pragma once

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Forwards
// ================================================================================================
enum VkIndexType;
enum VkFormat;
enum VkColorSpaceKHR;
enum VkAttachmentLoadOp;
enum VkAttachmentStoreOp;
enum VkPrimitiveTopology;
enum VkPolygonMode;
enum VkImageLayout;
enum VkDescriptorType;
enum VkFilter;
enum VkSamplerAddressMode;

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
uint32_t cydToVkShaderStages( CYD::ShaderStageFlag stages );
VkDescriptorType cydToVkDescriptorType( CYD::ShaderResourceType type );
VkFilter cydToVkFilter( CYD::Filter filter );
VkSamplerAddressMode cydToVkAddressMode( CYD::AddressMode mode );
VkImageLayout cydToVkImageLayout( CYD::ImageLayout layout );
}
