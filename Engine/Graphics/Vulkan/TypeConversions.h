#pragma once

#include <Common/Include.h>

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
namespace vk
{
class TypeConversions final
{
  public:
   TypeConversions() = delete;
   NON_COPIABLE( TypeConversions )
   ~TypeConversions() = delete;

   static VkIndexType cydToVkIndexType( CYD::IndexType type );
   static VkFormat cydToVkFormat( CYD::PixelFormat format );
   static VkColorSpaceKHR cydToVkSpace( CYD::ColorSpace space );
   static VkAttachmentLoadOp cydToVkOp( CYD::LoadOp op );
   static VkAttachmentStoreOp cydToVkOp( CYD::StoreOp op );
   static VkPrimitiveTopology cydToVkDrawPrim( CYD::DrawPrimitive prim );
   static VkPolygonMode cydToVkPolyMode( CYD::PolygonMode polyMode );
   static uint32_t cydToVkShaderStages( CYD::ShaderStageFlag stages );
   static VkDescriptorType cydToVkDescriptorType( CYD::ShaderResourceType type );
   static VkFilter cydToVkFilter( CYD::Filter filter );
   static VkSamplerAddressMode cydToVkAddressMode( CYD::AddressMode mode );
   static VkImageLayout cydToVkImageLayout( CYD::ImageLayout layout );
};
}
