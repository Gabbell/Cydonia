#include <Graphics/Vertex.h>

#include <Common/Assert.h>

namespace CYD
{
void VertexList::allocate( size_t vertexCount )
{
   CYD_ASSERT( m_vertices == nullptr );
   m_vertices    = static_cast<Vertex*>( malloc( vertexCount * m_layout.getStride() ) );
   m_vertexCount = static_cast<uint32_t>( vertexCount );
}

void VertexList::freeList()
{
   free( m_vertices );
   m_vertices    = nullptr;
   m_vertexCount = 0;
}

VertexList::~VertexList() { freeList(); }
}