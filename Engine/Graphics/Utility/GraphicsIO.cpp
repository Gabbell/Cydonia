#include <Graphics/Utility/GraphicsIO.h>

#include <Common/Assert.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/Vertex.h>

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

   std::vector<Vertex_PUNT> tempVertices;
   std::unordered_map<Vertex_PUNT, uint32_t> uniqueVertices = {};

   for( uint32_t s = 0; s < shapes.size(); ++s )
   {
      uint32_t indexOffset = 0;
      for( uint32_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f )
      {
         const uint32_t fv = shapes[s].mesh.num_face_vertices[f];
         CYD_ASSERT( fv == 3 && "Face did not have 3 vertices" );

         Vertex_PUNT face[3];

         bool hasNormals = false;
         for( uint32_t v = 0; v < fv; ++v )
         {
            Vertex_PUNT& vertex = face[v];

            const tinyobj::index_t& index = shapes[s].mesh.indices[indexOffset + v];

            // Position
            vertex.setAttribute<Vertex::Position>( glm::vec3(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2] ) );

            // UV (Texcoords)
            if( index.texcoord_index >= 0 )
            {
               vertex.setAttribute<Vertex::Texcoord>( glm::vec3(
                   attrib.texcoords[2 * index.texcoord_index + 0],
                   attrib.texcoords[2 * index.texcoord_index + 1],
                   0.0f ) );
            }

            // Normal
            if( index.normal_index >= 0 )
            {
               vertex.setAttribute<Vertex::Normal>( glm::vec3(
                   attrib.normals[3 * index.normal_index + 0],
                   attrib.normals[3 * index.normal_index + 1],
                   attrib.normals[3 * index.normal_index + 2] ) );

               hasNormals = true;
            }
         }

         if( hasNormals )
         {
            // Tangent
            const glm::vec3 deltaPos0 =
                face[1].getAttribute<Vertex::Position>() - face[0].getAttribute<Vertex::Position>();
            const glm::vec3 deltaPos1 =
                face[2].getAttribute<Vertex::Position>() - face[0].getAttribute<Vertex::Position>();
            const glm::vec3 deltaUV0 =
                face[1].getAttribute<Vertex::Texcoord>() - face[0].getAttribute<Vertex::Texcoord>();
            const glm::vec3 deltaUV1 =
                face[2].getAttribute<Vertex::Texcoord>() - face[0].getAttribute<Vertex::Texcoord>();

            const float denom = 1.0f / ( deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y );

            glm::vec3 tangent(
                denom * ( deltaUV1.y * deltaPos0.x - deltaUV0.y * deltaPos1.x ),
                denom * ( deltaUV1.y * deltaPos0.y - deltaUV0.y * deltaPos1.y ),
                denom * ( deltaUV1.y * deltaPos0.z - deltaUV0.y * deltaPos1.z ) );

            face[0].setAttribute<Vertex::Tangent>( tangent );
            face[1].setAttribute<Vertex::Tangent>( tangent );
            face[2].setAttribute<Vertex::Tangent>( tangent );
         }

         for( uint32_t v = 0; v < fv; ++v )
         {
            const Vertex_PUNT& vertex = face[v];
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
      const Vertex_PUNT& vertex = tempVertices[i];
      vertices.setValue<Vertex::Position>( i, vertex.getAttribute<Vertex::Position>() );
      vertices.setValue<Vertex::Texcoord>( i, vertex.getAttribute<Vertex::Texcoord>() );
      vertices.setValue<Vertex::Normal>( i, vertex.getAttribute<Vertex::Normal>() );
      vertices.setValue<Vertex::Tangent>( i, vertex.getAttribute<Vertex::Tangent>() );
   }
}

void* GraphicsIO::LoadImage(
    const std::string& path,
    PixelFormat format,
    int& width,
    int& height,
    int& size )
{
   CYD_TRACE_S( path );

   void* imageData = nullptr;
   int channels    = 0;

   switch( format )
   {
      case PixelFormat::RGBA32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGBA16F:
         imageData = stbi_load_16( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGBA8_UNORM:
      case PixelFormat::RGBA8_SRGB:
         imageData = stbi_load( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGB32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, STBI_rgb );
         break;
      case PixelFormat::R32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, STBI_grey );
         break;
      case PixelFormat::R16_UNORM:
         imageData = stbi_load_16( path.c_str(), &width, &height, &channels, STBI_grey );
         break;
      default:
         // TODO Format to pixel size function
         CYD_ASSERT( !"Not implemented" );
   }

   size = width * height * GetPixelSizeInBytes( format );

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
