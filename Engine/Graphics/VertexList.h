#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>

#include <glm/glm.hpp>

namespace CYD
{
class Vertex
{
  public:
   virtual ~Vertex() = default;

   struct Position
   {
      static constexpr VertexLayout::Attribute TYPE = VertexLayout::Attribute::POSITION;
      glm::vec3 data;
   };

   struct Texcoord
   {
      static constexpr VertexLayout::Attribute TYPE = VertexLayout::Attribute::TEXCOORD;
      glm::vec3 data;
   };

   struct Normal
   {
      static constexpr VertexLayout::Attribute TYPE = VertexLayout::Attribute::NORMAL;
      glm::vec3 data;
   };

   struct Tangent
   {
      static constexpr VertexLayout::Attribute TYPE = VertexLayout::Attribute::TANGENT;
      glm::vec3 data;
   };

   struct Color
   {
      static constexpr VertexLayout::Attribute TYPE = VertexLayout::Attribute::COLOR;
      glm::vec4 data;
   };

  protected:
   Vertex() = default;
   COPIABLE( Vertex );
};

class VertexList
{
  public:
   VertexList() = default;
   VertexList( const VertexLayout& layout ) : m_layout( layout ) {}
   MOVABLE( VertexList );
   ~VertexList();

   uint32_t getVertexCount() const { return m_vertexCount; }
   const Vertex* getData() const { return m_vertices; }
   size_t getSize() const { return m_vertexCount * m_layout.getStride(); }

   void setLayout( const VertexLayout& layout ) { m_layout = layout; }

   template <typename Attribute, typename Value>
   void setValue( uint32_t vertexIdx, const Value& newValue )
   {
      const size_t vertexOffset   = vertexIdx * m_layout.getStride();
      const size_t locationOffset = m_layout.getLocationOffset( Attribute::TYPE );

      if( locationOffset != VertexLayout::INVALID_LOCATION_OFFSET )
      {
         Value* value = reinterpret_cast<Value*>(
             reinterpret_cast<uint8_t*>( m_vertices ) + vertexOffset + locationOffset );

         *value = newValue;
      }
   }

   void allocate( size_t vertexCount );
   void freeList();

  private:
   VertexLayout m_layout;
   Vertex* m_vertices     = nullptr;
   uint32_t m_vertexCount = 0;
};
}