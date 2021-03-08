#include <Graphics/VertexLayout.h>

namespace CYD
{
void VertexLayout::addAttribute(
    PixelFormat vecFormat,
    uint32_t location,
    uint32_t offset,
    uint32_t binding )
{
   m_attributes.emplace_back( Attribute{ vecFormat, location, offset, binding } );
}
}