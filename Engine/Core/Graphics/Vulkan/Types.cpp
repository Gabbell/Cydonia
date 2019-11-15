#include <Core/Graphics/Vulkan/Types.h>

#include <Core/Common/Vulkan.h>
#include <Core/Common/Assert.h>

#include <functional>

// ================================================================================================
// Hashing functions

namespace cyd
{
// ================================================================================================
// Operator overloads
bool Extent::operator==( const Extent& other ) const
{
   return width == other.width && height == other.height;
}

bool Attachment::operator==( const Attachment& other ) const
{
   return format == other.format && loadOp == other.loadOp && storeOp == other.storeOp &&
          type == other.type && usage == other.usage;
}

bool RenderPassInfo::operator==( const RenderPassInfo& other ) const
{
   bool same = true;
   same      = attachments.size() == other.attachments.size();
   if( !same ) return false;

   for( uint32_t i = 0; i < attachments.size(); ++i )
   {
      same = attachments[i] == other.attachments[i];
      if( !same ) return false;
   }

   return true;
}

bool PushConstantRange::operator==( const PushConstantRange& other ) const
{
   return stages == other.stages && offset == other.offset && size == other.size;
}

bool ShaderObjectInfo::operator==( const ShaderObjectInfo& other ) const
{
   return stages == other.stages && binding == other.binding;
}

bool DescriptorSetLayoutInfo::operator==( const DescriptorSetLayoutInfo& other ) const
{
   return shaderObjects == other.shaderObjects;
}

bool PipelineLayoutInfo::operator==( const PipelineLayoutInfo& other ) const
{
   return ranges == other.ranges;
}

bool PipelineInfo::operator==( const PipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && drawPrim == other.drawPrim &&
          polyMode == other.polyMode && extent == other.extent && shaders == other.shaders;
}

bool SamplerInfo::operator==( const SamplerInfo& other ) const
{
   return useAnisotropy == other.useAnisotropy && maxAnisotropy == other.maxAnisotropy &&
          magFilter == other.magFilter && minFilter == other.minFilter &&
          addressMode == other.addressMode;
}

// ================================================================================================
// Conversion functions
VkFormat cydFormatToVkFormat( PixelFormat format )
{
   switch( format )
   {
      case cyd::PixelFormat::BGRA8_UNORM:
         return VK_FORMAT_B8G8R8A8_UNORM;
      default:
         CYDASSERT( !"Types: Pixel format not supported" );
   }

   return VK_FORMAT_B8G8R8A8_UNORM;
}

VkColorSpaceKHR cydSpaceToVkSpace( ColorSpace space )
{
   switch( space )
   {
      case cyd::ColorSpace::SRGB_NONLINEAR:
         return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      default:
         CYDASSERT( !"Types: Color space not supported" );
   }

   return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

VkAttachmentLoadOp cydOpToVkOp( LoadOp op )
{
   switch( op )
   {
      case cyd::LoadOp::CLEAR:
         return VK_ATTACHMENT_LOAD_OP_CLEAR;
      case cyd::LoadOp::LOAD:
         return VK_ATTACHMENT_LOAD_OP_LOAD;
      case cyd::LoadOp::DONT_CARE:
         return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      default:
         CYDASSERT( !"Types: Load operator not supported" );
   }
   return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp cydOpToVkOp( StoreOp op )
{
   switch( op )
   {
      case cyd::StoreOp::STORE:
         return VK_ATTACHMENT_STORE_OP_STORE;
      case cyd::StoreOp::DONT_CARE:
         return VK_ATTACHMENT_STORE_OP_DONT_CARE;
      default:
         CYDASSERT( !"Types: Store operator not supported" );
   }
   return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

VkPrimitiveTopology cydDrawPrimToVkDrawPrim( DrawPrimitive prim )
{
   switch( prim )
   {
      case cyd::DrawPrimitive::POINTS:
         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      case cyd::DrawPrimitive::LINES:
         return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      case cyd::DrawPrimitive::LINE_STRIPS:
         return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      case cyd::DrawPrimitive::TRIANGLES:
         return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      case cyd::DrawPrimitive::TRIANGLE_STRIPS:
         return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
      default:
         CYDASSERT( !"Types: Draw primitive not supported" );
   }
   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkPolygonMode cydPolyModeToVkPolyMode( PolygonMode polyMode )
{
   switch( polyMode )
   {
      case cyd::PolygonMode::FILL:
         return VK_POLYGON_MODE_FILL;
      case cyd::PolygonMode::LINE:
         return VK_POLYGON_MODE_LINE;
      case cyd::PolygonMode::POINT:
         return VK_POLYGON_MODE_POINT;
      default:
         CYDASSERT( !"Types: Polygon mode not supported" );
   }
   return VK_POLYGON_MODE_FILL;
}

VkShaderStageFlags cydShaderStagesToVkShaderStages( ShaderStageFlag stages )
{
   VkShaderStageFlags vkStages = 0;
   if( stages & ShaderStage::VERTEX_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_VERTEX_BIT;
   }
   if( stages & ShaderStage::FRAGMENT_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
   }
   if( stages & ShaderStage::COMPUTE_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_COMPUTE_BIT;
   }
   if( stages & ShaderStage::ALL_GRAPHICS_STAGES )
   {
      vkStages |= VK_SHADER_STAGE_ALL_GRAPHICS;
   }
   if( stages & ShaderStage::ALL_STAGES )
   {
      vkStages |= VK_SHADER_STAGE_ALL;
   }
   return vkStages;
}

VkImageLayout cydImageLayoutToVKImageLayout( ImageLayout layout )
{
   switch( layout )
   {
      case ImageLayout::UNDEFINED:
         return VK_IMAGE_LAYOUT_UNDEFINED;
      case ImageLayout::GENERAL:
         return VK_IMAGE_LAYOUT_GENERAL;
      case ImageLayout::COLOR:
         return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      case ImageLayout::PRESENTATION:
         return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      case ImageLayout::TRANSFER_SRC:
         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      case ImageLayout::TRANSFER_DST:
         return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      case ImageLayout::SHADER_READ:
         return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      default:
         CYDASSERT( !"Types: Image layout not supported" )
   }
   return VK_IMAGE_LAYOUT_GENERAL;
}

VkDescriptorType cydShaderObjectTypeToVkDescriptorType( ShaderObjectType type )
{
   switch( type )
   {
      case ShaderObjectType::UNIFORM:
         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      case ShaderObjectType::COMBINED_IMAGE_SAMPLER:
         return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      default:
         CYDASSERT( !"Types: Descriptor type not supported" );
   }

   return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

VkFilter cydFilterToVkFilter( Filter filter )
{
   switch( filter )
   {
      case Filter::NEAREST:
         return VK_FILTER_NEAREST;
      case Filter::LINEAR:
         return VK_FILTER_LINEAR;
      case Filter::CUBIC:
         return VK_FILTER_CUBIC_IMG;
      default:
         CYDASSERT( !"Types: Filter type not supported" );
   }

   return VK_FILTER_LINEAR;
}

VkSamplerAddressMode cydAddressModeToVkAddressMode( AddressMode mode )
{
   switch( mode )
   {
      case AddressMode::REPEAT:
         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
      case AddressMode::MIRRORED_REPEAT:
         return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
      case AddressMode::CLAMP_TO_EDGE:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      case AddressMode::CLAMP_TO_BORDER:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      case AddressMode::MIRROR_CLAMP_TO_EDGE:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      default:
         CYDASSERT( !"Types: Address mode not supported" );
   }

   return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}
}
