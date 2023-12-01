#include <Graphics/Utility/GraphicsIO.h>

#include <Common/Assert.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/VertexList.h>
#include <Graphics/VertexData.h>

#include <Profiling.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <algorithm>
#include <unordered_map>

namespace CYD
{
void GraphicsIO::LoadMesh(
    const std::string& path,
    VertexList& vertices,
    std::vector<uint32_t>& indices )
{
   CYD_TRACE_S( path );

   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;
   std::string warn, err;

   bool res =
       tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, ( path + ".obj" ).c_str() );
   CYD_ASSERT( res && "Model loading failed" );

   indices.reserve( shapes[0].mesh.indices.size() );

   std::vector<VertexData> tempVertices;
   std::unordered_map<VertexData, uint32_t> uniqueVertices = {};

   for( uint32_t s = 0; s < shapes.size(); ++s )
   {
      uint32_t indexOffset = 0;
      for( uint32_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f )
      {
         const uint32_t fv = shapes[s].mesh.num_face_vertices[f];
         CYD_ASSERT( fv == 3 && "Face did not have 3 vertices" );

         VertexData face[3];

         bool hasNormals = false;
         for( uint32_t v = 0; v < fv; ++v )
         {
            VertexData& vertex = face[v];

            const tinyobj::index_t& index = shapes[s].mesh.indices[indexOffset + v];

            // Position
            vertex.pos = glm::vec3(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2] );

            // UV (Texcoords)
            if( index.texcoord_index >= 0 )
            {
               vertex.uv = glm::vec3(
                   attrib.texcoords[2 * index.texcoord_index + 0],
                   attrib.texcoords[2 * index.texcoord_index + 1],
                   0.0f );
            }

            // Normal
            if( index.normal_index >= 0 )
            {
               vertex.normal = glm::vec3(
                   attrib.normals[3 * index.normal_index + 0],
                   attrib.normals[3 * index.normal_index + 1],
                   attrib.normals[3 * index.normal_index + 2] );

               hasNormals = true;
            }
         }

         if( hasNormals )
         {
            // Tangent
            const glm::vec3 deltaPos0 = face[1].pos - face[0].pos;
            const glm::vec3 deltaPos1 = face[2].pos - face[0].pos;
            const glm::vec3 deltaUV0  = face[1].uv - face[0].uv;
            const glm::vec3 deltaUV1  = face[2].uv - face[0].uv;

            const float denom = 1.0f / ( deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y );

            const glm::vec3 tangent(
                denom * ( deltaUV1.y * deltaPos0.x - deltaUV0.y * deltaPos1.x ),
                denom * ( deltaUV1.y * deltaPos0.y - deltaUV0.y * deltaPos1.y ),
                denom * ( deltaUV1.y * deltaPos0.z - deltaUV0.y * deltaPos1.z ) );

            face[0].tangent = tangent;
            face[1].tangent = tangent;
            face[2].tangent = tangent;
         }

         for( uint32_t v = 0; v < fv; ++v )
         {
            const VertexData& vertex = face[v];
            if( uniqueVertices.count( vertex ) == 0 )
            {
               uniqueVertices[vertex] = static_cast<uint32_t>( tempVertices.size() );
               tempVertices.push_back( vertex );
            }

            indices.push_back( uniqueVertices[vertex] );
         }

         indexOffset += fv;
      }
   }

   vertices.allocate( tempVertices.size() );
   for( uint32_t i = 0; i < tempVertices.size(); ++i )
   {
      const VertexData& vertex = tempVertices[i];
      vertices.setValue<Vertex::Position>( i, vertex.pos );
      vertices.setValue<Vertex::Texcoord>( i, vertex.uv );
      vertices.setValue<Vertex::Normal>( i, vertex.normal );
      vertices.setValue<Vertex::Tangent>( i, vertex.tangent );
   }
}

void* GraphicsIO::LoadImage(
    const std::string& path,
    PixelFormat format,
    uint32_t& width,
    uint32_t& height,
    uint32_t& size )
{
   CYD_TRACE_S( path );

   void* imageData = nullptr;
   int intWidth    = 0;
   int intHeight   = 0;
   int channels    = 0;

   switch( format )
   {
      case PixelFormat::RGBA32F:
         imageData = stbi_loadf( path.c_str(), &intWidth, &intHeight, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGBA16F:
         imageData = stbi_load_16( path.c_str(), &intWidth, &intHeight, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGBA8_UNORM:
      case PixelFormat::RGBA8_SRGB:
         imageData = stbi_load( path.c_str(), &intWidth, &intHeight, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGB32F:
         imageData = stbi_loadf( path.c_str(), &intWidth, &intHeight, &channels, STBI_rgb );
         break;
      case PixelFormat::R32F:
         imageData = stbi_loadf( path.c_str(), &intWidth, &intHeight, &channels, STBI_grey );
         break;
      case PixelFormat::R8_UNORM:
         imageData = stbi_load( path.c_str(), &intWidth, &intHeight, &channels, STBI_grey );
         break;
      case PixelFormat::R16_UNORM:
         imageData = stbi_load_16( path.c_str(), &intWidth, &intHeight, &channels, STBI_grey );
         break;
      case PixelFormat::R32_UINT:
         imageData = stbi_load( path.c_str(), &intWidth, &intHeight, &channels, STBI_grey );
         break;
      default:
         // TODO Format to pixel size function
         CYD_ASSERT( !"Not implemented" );
   }

   width  = intWidth;
   height = intHeight;
   size   = width * height * GetPixelSizeInBytes( format );

   if( !imageData )
   {
      // Could not load image, returning nullptr
      CYD_ASSERT( !"Could not load image" );
      return nullptr;
   }

   return imageData;
}

void GraphicsIO::FreeImage( void* imageData ) { stbi_image_free( imageData ); }
}