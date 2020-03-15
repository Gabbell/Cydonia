#include <ECS/Components/RenderableComponent.h>

#include <Common/Assert.h>

#include <Graphics/RenderInterface.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <array>
#include <unordered_map>

namespace cyd
{
// Placeholder texture
static constexpr uint32_t placeholderData[] = { 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF };
static constexpr TextureDescription placeholderDesc = {
    sizeof( placeholderData ),
    2,  // width
    2,  // height
    ImageType::TEXTURE_2D,
    PixelFormat::BGRA8_UNORM,
    ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED };

// Loading mesh using tinyobj
static void loadMesh(
    const std::string& meshPath,
    std::vector<Vertex>& vertices,
    std::vector<uint16_t>& indices )
{
   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;
   std::string warn, err;

   bool res = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, meshPath.c_str() );
   CYDASSERT( res && "Model loading failed" );

   vertices.reserve( shapes[0].mesh.num_face_vertices.size() );
   indices.reserve( shapes[0].mesh.indices.size() );

   std::unordered_map<Vertex, uint16_t> uniqueVertices = {};

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
            uniqueVertices[vertex] = static_cast<uint16_t>( vertices.size() );
            vertices.push_back( vertex );
         }

         indices.push_back( uniqueVertices[vertex] );
      }
   }
}

// Loading image using STB
static unsigned char*
loadImage( const std::string& imagePath, CmdListHandle transferList, TextureHandle* texHandle )
{
   int width, height, channels;
   width = height = channels = 0;

   // Loading albedo
   stbi_uc* imageData = stbi_load( imagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha );

   CYDASSERT( imageData && "RenderableComponent: Error happened while loading the image" );

   TextureDescription texDesc = {};
   texDesc.format             = PixelFormat::RGBA8_SRGB;
   texDesc.usage              = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
   texDesc.type               = ImageType::TEXTURE_2D;
   texDesc.width              = width;
   texDesc.height             = height;
   texDesc.size = width * height * sizeof( uint32_t );  // TODO Get pixel size from pixel format

   *texHandle = GRIS::CreateTexture( transferList, texDesc, imageData );

   return imageData;
}

// Vertex data constructor
RenderableComponent::RenderableComponent( const std::vector<Vertex>& vertices )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   albedo = GRIS::CreateTexture( transferList, placeholderDesc, placeholderData );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   vertexCount = static_cast<uint32_t>( vertices.size() );
   indexCount  = 0;

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}

// Mesh constructor
RenderableComponent::RenderableComponent( const std::string& meshPath )
{
   std::vector<Vertex> vertices;
   std::vector<uint16_t> indices;
   loadMesh( meshPath, vertices, indices );

   // Uploading data to GPU
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   albedo = GRIS::CreateTexture( transferList, placeholderDesc, placeholderData );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   vertexCount = static_cast<uint32_t>( vertices.size() );

   indexBuffer = GRIS::CreateIndexBuffer(
       transferList, static_cast<uint32_t>( indices.size() ), indices.data() );

   indexCount = static_cast<uint32_t>( indices.size() );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}

// PBR mesh constructor
RenderableComponent::RenderableComponent( const std::string& meshPath, const std::string& pbrPath )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   std::vector<Vertex> vertices;
   std::vector<uint16_t> indices;
   loadMesh( meshPath, vertices, indices );

   GRIS::StartRecordingCommandList( transferList );

   // Image data to free
   std::array<unsigned char*, 6> images = {};

   images[0] = loadImage( pbrPath + "albedo.png", transferList, &albedo );
   images[1] = loadImage( pbrPath + "normal.png", transferList, &normalMap );
   images[2] = loadImage( pbrPath + "metal.png", transferList, &metallicMap );
   images[3] = loadImage( pbrPath + "roughness.png", transferList, &roughnessMap );
   images[4] = loadImage( pbrPath + "ao.png", transferList, &ambientOcclusionMap );
   images[5] = loadImage( pbrPath + "height.png", transferList, &heightMap );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   vertexCount = static_cast<uint32_t>( vertices.size() );

   indexBuffer = GRIS::CreateIndexBuffer(
       transferList, static_cast<uint32_t>( indices.size() ), indices.data() );

   indexCount = static_cast<uint32_t>( indices.size() );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   for( const auto& image : images )
   {
      stbi_image_free( image );
   }
}

void RenderableComponent::uninit()
{
   GRIS::DestroyTexture( albedo );
   GRIS::DestroyTexture( normalMap );
   GRIS::DestroyTexture( metallicMap );
   GRIS::DestroyTexture( roughnessMap );
   GRIS::DestroyTexture( ambientOcclusionMap );
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
}
}
