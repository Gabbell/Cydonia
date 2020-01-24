#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Forwards
// ================================================================================================
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

   static VkFormat cydFormatToVkFormat( cyd::PixelFormat format );
   static VkColorSpaceKHR cydSpaceToVkSpace( cyd::ColorSpace space );
   static VkAttachmentLoadOp cydOpToVkOp( cyd::LoadOp op );
   static VkAttachmentStoreOp cydOpToVkOp( cyd::StoreOp op );
   static VkPrimitiveTopology cydDrawPrimToVkDrawPrim( cyd::DrawPrimitive prim );
   static VkPolygonMode cydPolyModeToVkPolyMode( cyd::PolygonMode polyMode );
   static uint32_t cydShaderStagesToVkShaderStages( cyd::ShaderStageFlag stages );
   static VkImageLayout cydImageLayoutToVKImageLayout( cyd::ImageLayout layout );
   static VkDescriptorType cydShaderObjectTypeToVkDescriptorType( cyd::ShaderResourceType type );
   static VkFilter cydFilterToVkFilter( cyd::Filter filter );
   static VkSamplerAddressMode cydAddressModeToVkAddressMode( cyd::AddressMode mode );
};
}
