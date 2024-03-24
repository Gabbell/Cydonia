#include <Graphics/GraphicsTypes.h>

#include <Common/Assert.h>

namespace CYD
{
bool IsColorFormat( PixelFormat format )
{
   if( format == PixelFormat::D32_SFLOAT )
   {
      return false;
   }

   return true;
}

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
      case PixelFormat::RGBA8_UNORM:
      case PixelFormat::RGBA8_SRGB:
      case PixelFormat::R32_UINT:
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
      default:
         CYD_ASSERT( !"Unknown pixel format" );
   }

   return 0;
}

uint32_t GetChannelsCount( PixelFormat format )
{
   switch( format )
   {
      case PixelFormat::R8_UNORM:
      case PixelFormat::R16_UNORM:
      case PixelFormat::R32_UINT:
      case PixelFormat::R32F:
      case PixelFormat::D32_SFLOAT:
         return 1;
      case PixelFormat::RG32F:
         return 2;
      case PixelFormat::RGB32F:
         return 3;
      case PixelFormat::BGRA8_UNORM:
      case PixelFormat::RGBA8_UNORM:
      case PixelFormat::RGBA8_SRGB:
      case PixelFormat::RGBA16F:
      case PixelFormat::RGBA32F:
         return 4;
      default:
         CYD_ASSERT( !"Unknown pixel format" );
   }

   return 0;
}

bool Extent2D::operator==( const Extent2D& other ) const
{
   return width == other.width && height == other.height;
}
}
