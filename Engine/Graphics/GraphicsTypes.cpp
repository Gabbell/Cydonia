#include <Graphics/GraphicsTypes.h>

namespace CYD
{
// Be aware that this size is minimal and should only be used for packed RAM allocations. It might
// not reflect the actual size that will be used by the graphics API to store this particular format
// on the device. Look at the API-specific memory related functions to query this information.
uint32_t GetPixelSizeInBytes( PixelFormat format )
{
   switch( format )
   {
      case PixelFormat::R8_UNORM:
         return 1;
      case PixelFormat::R16_UNORM:
         return 2;
      case PixelFormat::BGRA8_UNORM:
      case PixelFormat::RGBA8_SRGB:
      case PixelFormat::R32F:
      case PixelFormat::D32_SFLOAT:
         return 4;
      case PixelFormat::RGBA16F:
      case PixelFormat::RG32F:
         return 8;
      case PixelFormat::RGB32F:
         return 12;
      case PixelFormat::RGBA32F:
         return 16;
   }

   return 0;
}

bool Attachment::operator==( const Attachment& other ) const
{
   return format == other.format && loadOp == other.loadOp && storeOp == other.storeOp &&
          type == other.type;
}

bool RenderPassInfo::operator==( const RenderPassInfo& other ) const
{
   if( attachments.size() != other.attachments.size() ) return false;

   for( uint32_t i = 0; i < attachments.size(); ++i )
   {
      if( !( attachments[i] == other.attachments[i] ) ) return false;
   }

   return true;
}

bool Extent2D::operator==( const Extent2D& other ) const
{
   return width == other.width && height == other.height;
}
}
