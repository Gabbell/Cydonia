#include <Graphics/Vulkan/TypeConversions.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

namespace vk
{
VkIndexType TypeConversions::cydToVkIndexType( cyd::IndexType type )
{
   switch( type )
   {
      case cyd::IndexType::UNSIGNED_INT16:
         return VK_INDEX_TYPE_UINT16;
      case cyd::IndexType::UNSIGNED_INT32:
         return VK_INDEX_TYPE_UINT32;
      default:
         CYDASSERT( !"Types: Index type not supported" );
   }

   return VK_INDEX_TYPE_UINT16;
}

VkFormat TypeConversions::cydToVkFormat( cyd::PixelFormat format )
{
   switch( format )
   {
      case cyd::PixelFormat::BGRA8_UNORM:
         return VK_FORMAT_B8G8R8A8_UNORM;
      case cyd::PixelFormat::RGBA8_SRGB:
         return VK_FORMAT_R8G8B8A8_SRGB;
      case cyd::PixelFormat::RGBA16F_SFLOAT:
         return VK_FORMAT_R16G16B16A16_SFLOAT;
      case cyd::PixelFormat::RGBA32F_SFLOAT:
         return VK_FORMAT_R32G32B32A32_SFLOAT;
      case cyd::PixelFormat::D32_SFLOAT:
         return VK_FORMAT_D32_SFLOAT;
      default:
         CYDASSERT( !"Types: Pixel format not supported" );
   }

   return VK_FORMAT_B8G8R8A8_UNORM;
}

VkColorSpaceKHR TypeConversions::cydToVkSpace( cyd::ColorSpace space )
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

VkAttachmentLoadOp TypeConversions::cydToVkOp( cyd::LoadOp op )
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

VkAttachmentStoreOp TypeConversions::cydToVkOp( cyd::StoreOp op )
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

VkPrimitiveTopology TypeConversions::cydToVkDrawPrim( cyd::DrawPrimitive prim )
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

VkPolygonMode TypeConversions::cydToVkPolyMode( cyd::PolygonMode polyMode )
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

VkShaderStageFlags TypeConversions::cydToVkShaderStages( cyd::ShaderStageFlag stages )
{
   VkShaderStageFlags vkStages = 0;
   if( stages & cyd::ShaderStage::VERTEX_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_VERTEX_BIT;
   }
   if( stages & cyd::ShaderStage::FRAGMENT_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
   }
   if( stages & cyd::ShaderStage::COMPUTE_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_COMPUTE_BIT;
   }
   if( stages & cyd::ShaderStage::ALL_GRAPHICS_STAGES )
   {
      vkStages |= VK_SHADER_STAGE_ALL_GRAPHICS;
   }
   if( stages & cyd::ShaderStage::ALL_STAGES )
   {
      vkStages |= VK_SHADER_STAGE_ALL;
   }
   return vkStages;
}

VkDescriptorType TypeConversions::cydToVkDescriptorType(
    cyd::ShaderResourceType type )
{
   switch( type )
   {
      case cyd::ShaderResourceType::UNIFORM:
         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      case cyd::ShaderResourceType::COMBINED_IMAGE_SAMPLER:
         return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      default:
         CYDASSERT( !"Types: Descriptor type not supported" );
   }

   return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

VkFilter TypeConversions::cydToVkFilter( cyd::Filter filter )
{
   switch( filter )
   {
      case cyd::Filter::NEAREST:
         return VK_FILTER_NEAREST;
      case cyd::Filter::LINEAR:
         return VK_FILTER_LINEAR;
      case cyd::Filter::CUBIC:
         return VK_FILTER_CUBIC_IMG;
      default:
         CYDASSERT( !"Types: Filter type not supported" );
   }

   return VK_FILTER_LINEAR;
}

VkSamplerAddressMode TypeConversions::cydToVkAddressMode( cyd::AddressMode mode )
{
   switch( mode )
   {
      case cyd::AddressMode::REPEAT:
         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
      case cyd::AddressMode::MIRRORED_REPEAT:
         return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
      case cyd::AddressMode::CLAMP_TO_EDGE:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      case cyd::AddressMode::CLAMP_TO_BORDER:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      case cyd::AddressMode::MIRROR_CLAMP_TO_EDGE:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      default:
         CYDASSERT( !"Types: Address mode not supported" );
   }

   return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}
}
