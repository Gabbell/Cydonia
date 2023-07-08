#include <Graphics/Vulkan/TypeConversions.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>

namespace vk::TypeConversions
{
VkIndexType cydToVkIndexType( CYD::IndexType type )
{
   switch( type )
   {
      case CYD::IndexType::UNSIGNED_INT16:
         return VK_INDEX_TYPE_UINT16;
      case CYD::IndexType::UNSIGNED_INT32:
         return VK_INDEX_TYPE_UINT32;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized index type" );
   return VK_INDEX_TYPE_UINT16;
}

VkFormat cydToVkFormat( CYD::PixelFormat format )
{
   switch( format )
   {
      case CYD::PixelFormat::BGRA8_UNORM:
         return VK_FORMAT_B8G8R8A8_UNORM;
      case CYD::PixelFormat::RGBA8_UNORM:
         return VK_FORMAT_R8G8B8A8_UNORM;
      case CYD::PixelFormat::RGBA8_SRGB:
         return VK_FORMAT_R8G8B8A8_SRGB;
      case CYD::PixelFormat::RGBA16F:
         return VK_FORMAT_R16G16B16A16_SFLOAT;
      case CYD::PixelFormat::RGBA32F:
         return VK_FORMAT_R32G32B32A32_SFLOAT;
      case CYD::PixelFormat::RGB32F:
         return VK_FORMAT_R32G32B32_SFLOAT;
      case CYD::PixelFormat::RG32F:
         return VK_FORMAT_R32G32_SFLOAT;
      case CYD::PixelFormat::R32F:
         return VK_FORMAT_R32_SFLOAT;
      case CYD::PixelFormat::D32_SFLOAT:
         return VK_FORMAT_D32_SFLOAT;
      case CYD::PixelFormat::R16_UNORM:
         return VK_FORMAT_R16_UNORM;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized pixel format" );
   return VK_FORMAT_B8G8R8A8_UNORM;
}

CYD::PixelFormat vkToCydFormat( VkFormat format )
{
   switch( format )
   {
      case VK_FORMAT_B8G8R8A8_UNORM:
         return CYD::PixelFormat::BGRA8_UNORM;
      case VK_FORMAT_R8G8B8A8_UNORM:
         return CYD::PixelFormat::RGBA8_UNORM;
      case VK_FORMAT_R8G8B8A8_SRGB:
         return CYD::PixelFormat::RGBA8_SRGB;
      case VK_FORMAT_R16G16B16A16_SFLOAT:
         return CYD::PixelFormat::RGBA16F;
      case VK_FORMAT_R32G32B32A32_SFLOAT:
         return CYD::PixelFormat::RGBA32F;
      case VK_FORMAT_R32G32B32_SFLOAT:
         return CYD::PixelFormat::RGB32F;
      case VK_FORMAT_R32G32_SFLOAT:
         return CYD::PixelFormat::RG32F;
      case VK_FORMAT_R32_SFLOAT:
         return CYD::PixelFormat::R32F;
      case VK_FORMAT_D32_SFLOAT:
         return CYD::PixelFormat::D32_SFLOAT;
      case VK_FORMAT_R16_UNORM:
         return CYD::PixelFormat::R16_UNORM;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized pixel format" );
   return CYD::PixelFormat::BGRA8_UNORM;
}

VkColorSpaceKHR cydToVkSpace( CYD::ColorSpace space )
{
   switch( space )
   {
      case CYD::ColorSpace::SRGB_NONLINEAR:
         return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized color space" );
   return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

VkAttachmentLoadOp cydToVkOp( CYD::LoadOp op )
{
   switch( op )
   {
      case CYD::LoadOp::CLEAR:
         return VK_ATTACHMENT_LOAD_OP_CLEAR;
      case CYD::LoadOp::LOAD:
         return VK_ATTACHMENT_LOAD_OP_LOAD;
      case CYD::LoadOp::DONT_CARE:
         return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized load operator" );
   return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp cydToVkOp( CYD::StoreOp op )
{
   switch( op )
   {
      case CYD::StoreOp::STORE:
         return VK_ATTACHMENT_STORE_OP_STORE;
      case CYD::StoreOp::DONT_CARE:
         return VK_ATTACHMENT_STORE_OP_DONT_CARE;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized store operator" );
   return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

VkPrimitiveTopology cydToVkDrawPrim( CYD::DrawPrimitive prim )
{
   switch( prim )
   {
      case CYD::DrawPrimitive::POINTS:
         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      case CYD::DrawPrimitive::LINES:
         return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      case CYD::DrawPrimitive::LINE_STRIPS:
         return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      case CYD::DrawPrimitive::TRIANGLES:
         return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      case CYD::DrawPrimitive::TRIANGLE_STRIPS:
         return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized draw primitive" );
   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkPolygonMode cydToVkPolyMode( CYD::PolygonMode polyMode )
{
   switch( polyMode )
   {
      case CYD::PolygonMode::FILL:
         return VK_POLYGON_MODE_FILL;
      case CYD::PolygonMode::LINE:
         return VK_POLYGON_MODE_LINE;
      case CYD::PolygonMode::POINT:
         return VK_POLYGON_MODE_POINT;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized polygon mode" );
   return VK_POLYGON_MODE_FILL;
}

VkPipelineStageFlags cydToVkPipelineStages( CYD::PipelineStageFlag stages )
{
   VkPipelineStageFlags vkStages = 0;
   if( stages & CYD::PipelineStage::VERTEX_STAGE )
   {
      vkStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
   }
   if( stages & CYD::PipelineStage::FRAGMENT_STAGE )
   {
      vkStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   }
   if( stages & CYD::PipelineStage::COMPUTE_STAGE )
   {
      vkStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   }
   if( stages & CYD::PipelineStage::TRANSFER_STAGE )
   {
      vkStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
   }
   if( stages & CYD::PipelineStage::ALL_GRAPHICS_STAGES )
   {
      vkStages |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
   }
   if( stages & CYD::PipelineStage::ALL_STAGES )
   {
      vkStages |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
   }

   return vkStages;
}

VkShaderStageFlags cydToVkShaderStages( CYD::PipelineStageFlag stages )
{
   // Invalid stages for shaders
   CYD_ASSERT( !( stages & CYD::PipelineStage::TRANSFER_STAGE ) );

   VkShaderStageFlags vkStages = 0;
   if( stages & CYD::PipelineStage::VERTEX_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_VERTEX_BIT;
   }
   if( stages & CYD::PipelineStage::FRAGMENT_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
   }
   if( stages & CYD::PipelineStage::COMPUTE_STAGE )
   {
      vkStages |= VK_SHADER_STAGE_COMPUTE_BIT;
   }
   if( stages & CYD::PipelineStage::ALL_GRAPHICS_STAGES )
   {
      vkStages |= VK_SHADER_STAGE_ALL_GRAPHICS;
   }
   if( stages & CYD::PipelineStage::ALL_STAGES )
   {
      vkStages |= VK_SHADER_STAGE_ALL;
   }

   return vkStages;
}

VkDescriptorType cydToVkDescriptorType( CYD::ShaderResourceType type )
{
   switch( type )
   {
      case CYD::ShaderResourceType::UNIFORM:
         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      case CYD::ShaderResourceType::STORAGE:
         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      case CYD::ShaderResourceType::COMBINED_IMAGE_SAMPLER:
         return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      case CYD::ShaderResourceType::STORAGE_IMAGE:
         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      case CYD::ShaderResourceType::SAMPLED_IMAGE:
         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized shader resource type" );
   return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

VkFilter cydToVkFilter( CYD::Filter filter )
{
   switch( filter )
   {
      case CYD::Filter::NEAREST:
         return VK_FILTER_NEAREST;
      case CYD::Filter::LINEAR:
         return VK_FILTER_LINEAR;
      case CYD::Filter::CUBIC:
         return VK_FILTER_CUBIC_IMG;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized filter" );
   return VK_FILTER_LINEAR;
}

VkBorderColor cydToVkBorderColor( CYD::BorderColor borderColor )
{
   switch( borderColor )
   {
      case CYD::BorderColor::OPAQUE_BLACK:
         return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
      case CYD::BorderColor::OPAQUE_WHITE:
         return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
      case CYD::BorderColor::TRANSPARENT_BLACK:
         return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized border color" );
   return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
}

VkSamplerAddressMode cydToVkAddressMode( CYD::AddressMode mode )
{
   switch( mode )
   {
      case CYD::AddressMode::REPEAT:
         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
      case CYD::AddressMode::MIRRORED_REPEAT:
         return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
      case CYD::AddressMode::CLAMP_TO_EDGE:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      case CYD::AddressMode::CLAMP_TO_BORDER:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      case CYD::AddressMode::MIRROR_CLAMP_TO_EDGE:
         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized addressing mode" );
   return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkImageLayout cydToVkImageLayout( CYD::ImageLayout layout )
{
   switch( layout )
   {
      case CYD::ImageLayout::UNKNOWN:
         return VK_IMAGE_LAYOUT_UNDEFINED;
      case CYD::ImageLayout::GENERAL:
         return VK_IMAGE_LAYOUT_GENERAL;
      case CYD::ImageLayout::TRANSFER_SRC:
         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      case CYD::ImageLayout::TRANSFER_DST:
         return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      case CYD::ImageLayout::COLOR_ATTACHMENT:
         return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      case CYD::ImageLayout::DEPTH_ATTACHMENT:
         return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
      case CYD::ImageLayout::DEPTH_STENCIL_ATTACHMENT:
         return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      case CYD::ImageLayout::DEPTH_STENCIL_READ:
         return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      case CYD::ImageLayout::PRESENT_SRC:
         return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      case CYD::ImageLayout::SHADER_READ:
         return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized image layout" );
   return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkCompareOp cydToVkCompareOp( CYD::CompareOperator op )
{
   switch( op )
   {
      case CYD::CompareOperator::NEVER:
         return VK_COMPARE_OP_NEVER;
         ;
      case CYD::CompareOperator::LESS:
         return VK_COMPARE_OP_LESS;
      case CYD::CompareOperator::EQUAL:
         return VK_COMPARE_OP_EQUAL;
      case CYD::CompareOperator::LESS_EQUAL:
         return VK_COMPARE_OP_LESS_OR_EQUAL;
      case CYD::CompareOperator::GREATER:
         return VK_COMPARE_OP_GREATER;
      case CYD::CompareOperator::GREATER_EQUAL:
         return VK_COMPARE_OP_GREATER_OR_EQUAL;
      case CYD::CompareOperator::NOT_EQUAL:
         return VK_COMPARE_OP_NOT_EQUAL;
      case CYD::CompareOperator::ALWAYS:
         return VK_COMPARE_OP_ALWAYS;
   }

   CYD_ASSERT( !"TypeConversions:: Unrecognized compare operator" );
   return VK_COMPARE_OP_ALWAYS;
}

VkImageAspectFlags getAspectMask( CYD::PixelFormat format )
{
   switch( format )
   {
      case CYD::PixelFormat::D32_SFLOAT:
         return VK_IMAGE_ASPECT_DEPTH_BIT;
      default:
         return VK_IMAGE_ASPECT_COLOR_BIT;
   }
}
}