#include <Graphics/MeshGeneration.h>

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
   indices.resize( 3 * ( rows * columns ) );

   // Vertices
   for( uint32_t r = 0; r < rows; ++r )
   {
      for( uint32_t c = 0; c < columns; ++c )
      {
         const uint32_t index   = ( r * columns ) + c;
         vertices[index].pos    = glm::vec3( c, r, 0.0f );
         vertices[index].col    = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
         vertices[index].normal = glm::vec3( 0.0, 1.0f, 0.0f );
      }
   }

   // Indices
   uint32_t i = 0;
   for( uint32_t r = 0; r < rows - 1; ++r )
   {
      indices[i++] = r * columns;
      for( uint32_t c = 0; c < columns; ++c )
      {
         indices[i++] = ( r * columns ) + c;
         indices[i++] = ( r + 1 ) * columns + c;
      }
      indices[i++] = ( r + 1 ) * columns + ( columns - 1 );
   }
}
}
