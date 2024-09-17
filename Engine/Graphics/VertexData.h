#pragma once

#include <Common/Include.h>

#include <glm/glm.hpp>

namespace CYD
{
// Hashable internal vertex data struct used to allocate temporary vertex memory and compare with
// other vertices
struct VertexData final
{
   VertexData() { memset( this, 0, sizeof( VertexData ) ); }

   bool operator==( const VertexData& other ) const
   {
      return pos == other.pos && col == other.col && normal == other.normal && uv == other.uv;
   }

   glm::vec3 pos;
   glm::vec3 uv;
   glm::vec3 normal;
   glm::vec3 tangent;
   glm::vec4 col;
};
}

template <>
struct ::std::hash<CYD::VertexData>
{
   size_t operator()( const CYD::VertexData& vertex ) const noexcept

   {
      size_t seed = 0;
      hashCombine( seed, vertex.pos.x );
      hashCombine( seed, vertex.pos.y );
      hashCombine( seed, vertex.pos.z );
      hashCombine( seed, vertex.uv.x );
      hashCombine( seed, vertex.uv.y );
      hashCombine( seed, vertex.normal.x );
      hashCombine( seed, vertex.normal.y );
      hashCombine( seed, vertex.normal.z );
      hashCombine( seed, vertex.tangent.x );
      hashCombine( seed, vertex.tangent.y );
      hashCombine( seed, vertex.tangent.z );
      hashCombine( seed, vertex.col.r );
      hashCombine( seed, vertex.col.g );
      hashCombine( seed, vertex.col.b );
      hashCombine( seed, vertex.col.a );
      return seed;
   }
};