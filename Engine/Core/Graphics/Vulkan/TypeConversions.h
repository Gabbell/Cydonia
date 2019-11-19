#pragma once

#include <Core/Graphics/Vulkan/Types.h>

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
namespace cyd
{
class TypeConversions
{
  public:
   TypeConversions()  = delete;
   ~TypeConversions() = delete;

   static VkFormat cydFormatToVkFormat( PixelFormat format );
   static VkColorSpaceKHR cydSpaceToVkSpace( ColorSpace space );
   static VkAttachmentLoadOp cydOpToVkOp( LoadOp op );
   static VkAttachmentStoreOp cydOpToVkOp( StoreOp op );
   static VkPrimitiveTopology cydDrawPrimToVkDrawPrim( DrawPrimitive prim );
   static VkPolygonMode cydPolyModeToVkPolyMode( PolygonMode polyMode );
   static uint32_t cydShaderStagesToVkShaderStages( ShaderStageFlag stages );
   static VkImageLayout cydImageLayoutToVKImageLayout( ImageLayout layout );
   static VkDescriptorType cydShaderObjectTypeToVkDescriptorType( ShaderObjectType type );
   static VkFilter cydFilterToVkFilter( Filter filter );
   static VkSamplerAddressMode cydAddressModeToVkAddressMode( AddressMode mode );
};
}
