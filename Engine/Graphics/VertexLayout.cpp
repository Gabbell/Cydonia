#include <Graphics/VertexLayout.h>

#include <Common/Assert.h>

namespace CYD
{
void InitializeVertexLayouts()
{
   //
}

uint32_t VertexLayout::getLocationOffset( Attribute type ) const
{
   for( const auto& attribute : m_attributes )
   {
      if( attribute.type == type )
      {
         return attribute.offset;
      }
   }

   return INVALID_LOCATION_OFFSET;
}

void VertexLayout::addAttribute( Attribute type, PixelFormat vecFormat )
{
   m_attributes.emplace_back( AttributeInfo{ type, vecFormat, m_stride } );
   m_stride += GetPixelSizeInBytes( vecFormat );
}
}