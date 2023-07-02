#include <Graphics/Utility/GraphicsIO.h>

#include <Common/Assert.h>

#include <Graphics/VertexLayout.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <unordered_map>

namespace CYD
{
static const char MESH_PATH[] = "../Engine/Data/Meshes/";

void GraphicsIO::LoadMesh(
    const std::string& path,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices )
{
   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;
   std::string warn, err;

   bool res = tinyobj::LoadObj(
       &attrib, &shapes, &materials, &warn, &err, ( MESH_PATH + path + ".obj" ).c_str() );
   CYD_ASSERT( res && "Model loading failed" );

   vertices.reserve( shapes[0].mesh.num_face_vertices.size() );
   indices.reserve( shapes[0].mesh.indices.size() );

   std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

   for( const auto& shape : shapes )
   {
      for( const auto& index : shape.mesh.indices )
      {
         Vertex vertex = {};

         vertex.pos = {
             attrib.vertices[3 * index.vertex_index + 0],
             attrib.vertices[3 * index.vertex_index + 1],
             attrib.vertices[3 * index.vertex_index + 2] };

         if( !attrib.normals.empty() )
         {
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2] };
         }

         if( !attrib.texcoords.empty() )
         {
            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                0.0f };
         }

         vertex.col = { 1.0f, 1.0f, 1.0f, 1.0f };

         if( uniqueVertices.count( vertex ) == 0 )
         {
            uniqueVertices[vertex] = static_cast<uint32_t>( vertices.size() );
            vertices.push_back( vertex );
         }

         indices.push_back( uniqueVertices[vertex] );
      }
   }
}

void* GraphicsIO::LoadImage(
    const std::string& path,
    PixelFormat format,
    int& width,
    int& height,
    int& size )
{
   void* imageData = nullptr;
   int channels    = 0;
   switch( format )
   {
      case PixelFormat::RGBA32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::RGB32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, STBI_rgb );
         break;
      case PixelFormat::RGBA8_SRGB:
         imageData = stbi_load( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         break;
      case PixelFormat::R32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, 0 );
         break;
      default:
         // TODO Format to pixel size function
         CYD_ASSERT( !"Not implemented" );
   }

   size = width * height * GetPixelSizeInBytes( format );

   if( !imageData )
   {
      // Could not load image, returning nullptr
      return nullptr;
   }

   return imageData;
}

void GraphicsIO::FreeImage( void* imageData ) { stbi_image_free( imageData ); }
}
