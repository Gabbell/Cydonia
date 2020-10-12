#include <Graphics/Utility/MeshGeneration.h>

#include <Graphics/GraphicsTypes.h>

namespace CYD::MeshGen
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
             c - ( static_cast<float>( columns ) / 2.0f ),
             0.0f,
             r - static_cast<float>( rows ) / 2.0f );
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
}
