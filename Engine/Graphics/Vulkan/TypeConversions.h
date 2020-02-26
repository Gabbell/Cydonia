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

   static VkIndexType cydToVkIndexType( cyd::IndexType type );
   static VkFormat cydToVkFormat( cyd::PixelFormat format );
   static VkColorSpaceKHR cydToVkSpace( cyd::ColorSpace space );
   static VkAttachmentLoadOp cydToVkOp( cyd::LoadOp op );
   static VkAttachmentStoreOp cydToVkOp( cyd::StoreOp op );
   static VkPrimitiveTopology cydToVkDrawPrim( cyd::DrawPrimitive prim );
   static VkPolygonMode cydToVkPolyMode( cyd::PolygonMode polyMode );
   static uint32_t cydToVkShaderStages( cyd::ShaderStageFlag stages );
   static VkImageLayout cydToVkImageLayout( cyd::ImageLayout layout );
   static VkDescriptorType cydToVkDescriptorType( cyd::ShaderResourceType type );
   static VkFilter cydToVkFilter( cyd::Filter filter );
   static VkSamplerAddressMode cydToVkAddressMode( cyd::AddressMode mode );
};
}
