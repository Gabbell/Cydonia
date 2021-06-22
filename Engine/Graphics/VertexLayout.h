#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <glm/glm.hpp>

namespace CYD
{
class VertexLayout
{
  public:
   VertexLayout() = default;
   COPIABLE( VertexLayout );
   virtual ~VertexLayout() = default;

   struct Attribute
   {
      // Not used as a pixel format but as a sizing factor for vector components
      PixelFormat vecFormat = PixelFormat::RGBA32F;
      uint32_t location     = 0;
      uint32_t offset       = 0;
      uint32_t binding      = 0;
   };

   void addAttribute(
       PixelFormat vecFormat,
       uint32_t location = 0,
       uint32_t offset   = 0,
       uint32_t binding  = 0 );

   const std::vector<Attribute>& getAttributes() const { return m_attributes; }

  private:
   std::vector<Attribute> m_attributes;
};

class Vertex final
{
  public:
   Vertex() = default;
   explicit Vertex( const glm::vec3& position ) : pos( position ) {}
   COPIABLE( Vertex );
   ~Vertex() = default;

   bool operator==( const Vertex& other ) const
   {
      return pos == other.pos && col == other.col && uv == other.uv;
   }

   glm::vec3 pos;
   glm::vec4 col = glm::vec4( 1.0f );
   glm::vec3 uv;
   glm::vec3 normal;
};
}

template <>
struct std::hash<CYD::Vertex>
{
   size_t operator()( const CYD::Vertex& vertex ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, vertex.pos.x );
      hashCombine( seed, vertex.pos.y );
      hashCombine( seed, vertex.pos.z );
      hashCombine( seed, vertex.col.r );
      hashCombine( seed, vertex.col.g );
      hashCombine( seed, vertex.col.b );
      hashCombine( seed, vertex.col.a );
      hashCombine( seed, vertex.uv.x );
      hashCombine( seed, vertex.uv.y );

      return seed;
   }
};