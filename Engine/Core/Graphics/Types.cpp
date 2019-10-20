#include <Core/Graphics/Types.h>

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

bool PipelineLayoutInfo::operator==( const PipelineLayoutInfo& other ) const
{
   return dummy == other.dummy;
}

bool PipelineInfo::operator==( const PipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && drawPrim == other.drawPrim &&
          polyMode == other.polyMode && extent == other.extent && shaders == other.shaders;
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
         VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      case cyd::DrawPrimitive::LINES:
         return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      case cyd::DrawPrimitive::LINE_STRIPS:
         return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      case cyd::DrawPrimitive::TRIANGLES:
         return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      case cyd::DrawPrimitive::TRIANGLE_STRIPS:
         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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
}
