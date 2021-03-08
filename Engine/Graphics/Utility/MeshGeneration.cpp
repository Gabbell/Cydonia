#include <Graphics/Utility/MeshGeneration.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>

#include <unordered_map>

namespace CYD::MeshGeneration
{
void Grid(
    uint32_t rows,
    uint32_t columns,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices )
{
   vertices.resize( rows * columns );

   // Vertices in triangles
   for( uint32_t r = 0; r < rows; ++r )
   {
      for( uint32_t c = 0; c < columns; ++c )
      {
         const uint32_t index = ( r * columns ) + c;
         vertices[index].pos  = glm::vec3(
             ( static_cast<float>( c ) / static_cast<float>( columns ) ) - 0.5f,
             0.0f,
             ( static_cast<float>( r ) / static_cast<float>( rows ) ) - 0.5f );
         vertices[index].uv =
             glm::vec3( static_cast<float>( c ) / columns, static_cast<float>( r ) / rows, 0.0f );
         vertices[index].col    = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
         vertices[index].normal = glm::vec3( 0.0, 1.0f, 0.0f );
      }
   }

   // Indices in quads (hence the -1)
   for( uint32_t r = 0; r < ( rows - 1 ); ++r )
   {
      for( uint32_t c = 0; c < ( columns - 1 ); ++c )
      {
         indices.push_back( c + ( r * columns ) );
         indices.push_back( c + ( ( r + 1 ) * columns ) );
         indices.push_back( c + ( ( r + 1 ) * columns ) + 1 );

         indices.push_back( c + ( r * columns ) );
         indices.push_back( c + ( ( r + 1 ) * columns ) + 1 );
         indices.push_back( c + ( r * columns + 1 ) );
      }
   }
}

// Recursive subdivision function for icosphere generation
static void subdivide(
    std::unordered_map<Vertex, uint32_t>& uniqueVertices,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    const glm::vec3& p0,
    const glm::vec3& p1,
    const glm::vec3& p2,
    uint32_t subdivision,
    float divisionPerEdge )
{
   if( subdivision == 0 )
   {
      const Vertex v0( p0 );
      const Vertex v1( p1 );
      const Vertex v2( p2 );

      // We're at the lowest level of subdivision, add the vertices
      if( uniqueVertices.count( v0 ) == 0 )
      {
         uniqueVertices[v0] = static_cast<uint32_t>( vertices.size() );
         vertices.push_back( v0 );
      }
      if( uniqueVertices.count( v1 ) == 0 )
      {
         uniqueVertices[v1] = static_cast<uint32_t>( vertices.size() );
         vertices.push_back( v1 );
      }
      if( uniqueVertices.count( v2 ) == 0 )
      {
         uniqueVertices[v2] = static_cast<uint32_t>( vertices.size() );
         vertices.push_back( v2 );
      }

      indices.push_back( uniqueVertices[v0] );
      indices.push_back( uniqueVertices[v1] );
      indices.push_back( uniqueVertices[v2] );

      return;
   }

   // Projecting onto a sphere by normalizing the vectors so that they all have a distance of 1 from
   // the origin of the sphere
   const glm::vec3 n0 = glm::normalize( p0 );
   const glm::vec3 n1 = glm::normalize( p1 );
   const glm::vec3 n2 = glm::normalize( p2 );

   // The division factor to get this many points per edge per subdivision
   const glm::vec3 points[6] = {
       n0,
       n1,
       n2,
       ( n0 + n1 ) / divisionPerEdge,
       ( n1 + n2 ) / divisionPerEdge,
       ( n2 + n0 ) / divisionPerEdge };

   static constexpr uint32_t idx[12] = { 0, 3, 5, 5, 3, 4, 3, 1, 4, 5, 4, 2 };

   for( uint32_t i = 0; i < 4; ++i )
   {
      subdivide(
          uniqueVertices,
          vertices,
          indices,
          points[idx[3 * i + 0]],
          points[idx[3 * i + 1]],
          points[idx[3 * i + 2]],
          subdivision - 1,
          divisionPerEdge );
   }
}

void Icosphere(
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    uint32_t subdivisions,
    float divisionPerEdge )
{
   // Golden ratio
   static constexpr float ratio = 1.61803398875f;

   static constexpr std::array<glm::vec3, 12> icosahedronVerts = { {
       { -1, ratio, 0 },
       { 1, ratio, 0 },
       { -1, -ratio, 0 },
       { 1, -ratio, 0 },
       { 0, -1, ratio },
       { 0, 1, ratio },
       { 0, -1, -ratio },
       { 0, 1, -ratio },
       { ratio, 0, -1 },
       { ratio, 0, 1 },
       { -ratio, 0, -1 },
       { -ratio, 0, 1 },
   } };

   static constexpr std::array<uint32_t, 63> icosahedronIdx = {
       { 0,  11, 5, 0,  5, 1, 0,  1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4,
         11, 10, 2, 10, 7, 6, 10, 7, 6, 7, 1, 8,  3, 9,  4,  3, 4, 2, 3, 2,  6,
         3,  6,  8, 3,  8, 9, 4,  9, 5, 2, 4, 11, 6, 2,  10, 8, 6, 7, 9, 8,  1 } };

   std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

   for( uint32_t i = 0; i < 21; ++i )
   {
      // Subdivision using recursion
      subdivide(
          uniqueVertices,
          vertices,
          indices,
          glm::normalize( icosahedronVerts[icosahedronIdx[i * 3 + 0]] ),
          glm::normalize( icosahedronVerts[icosahedronIdx[i * 3 + 1]] ),
          glm::normalize( icosahedronVerts[icosahedronIdx[i * 3 + 2]] ),
          subdivisions,
          divisionPerEdge );
   }
}
}
