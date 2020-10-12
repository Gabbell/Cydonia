#include <Graphics/Utility/GraphicsIO.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <unordered_map>

namespace CYD
{
static const char MESH_PATH[] = "Data/Meshes/";

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
   CYDASSERT( res && "Model loading failed" );

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

         vertex.normal = {
             attrib.normals[3 * index.normal_index + 0],
             attrib.normals[3 * index.normal_index + 1],
             attrib.normals[3 * index.normal_index + 2] };

         vertex.uv = {
             attrib.texcoords[2 * index.texcoord_index + 0],
             1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
             0.0f };

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

void* GraphicsIO::LoadImage( const TextureDescription& desc, const std::string& path )
{
   size_t imageSize = 0;
   int width, height, channels;
   width = height = channels = 0;

   void* imageData = nullptr;
   switch( desc.format )
   {
      case PixelFormat::RGBA32F:
         imageData = stbi_loadf( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         imageSize = width * height * sizeof( float ) * 4;
         break;
      default:
         imageData = stbi_load( path.c_str(), &width, &height, &channels, STBI_rgb_alpha );
         imageSize = width * height * sizeof( uint32_t );  // TODO Format to pixel size function
   }

   if( !imageData )
   {
      // Could not load image, returning nullptr
      return nullptr;
   }

   if( imageSize != ( desc.size / desc.layers ) || static_cast<uint32_t>( width ) != desc.width ||
       static_cast<uint32_t>( height ) != desc.height )
   {
      CYDASSERT( !"GraphicsIO: Mismatch with texture description and actual image" );
   }

   return imageData;
}

void GraphicsIO::FreeImage( void* imageData ) { stbi_image_free( imageData ); }
}
