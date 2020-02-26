#include <ECS/Components/RenderableComponent.h>

#include <Common/Assert.h>

#include <Graphics/RenderInterface.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <array>
#include <unordered_map>

namespace cyd
{
RenderableComponent::RenderableComponent( const std::string& meshPath )
{
   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;
   std::string warn, err;

   bool res = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, meshPath.c_str() );
   CYDASSERT( res && "Model loading failed" );

   std::unordered_map<Vertex, uint16_t> uniqueVertices = {};
   std::vector<Vertex> vertices;
   std::vector<uint16_t> indices;

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

   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   matBuffer = GRIS::CreateUniformBuffer( sizeof( glm::vec4 ) );

   // Placeholder texture
   std::array<uint32_t, 4> texData = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF};
   const size_t texSize            = sizeof( texData[0] ) * texData.size();

   TextureDescription texDesc;
   texDesc.size   = texSize;
   texDesc.width  = 2;
   texDesc.height = 2;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   GRIS::StartRecordingCommandList( transferList );

   matTexture = GRIS::CreateTexture( transferList, texDesc, texData.data() );

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

RenderableComponent::RenderableComponent( const std::vector<Vertex>& vertices )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   matBuffer = GRIS::CreateUniformBuffer( sizeof( glm::vec4 ) );

   // Placeholder texture
   std::array<uint32_t, 4> texData = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF };
   const size_t texSize            = sizeof( texData[0] ) * texData.size();

   TextureDescription texDesc;
   texDesc.size   = texSize;
   texDesc.width  = 2;
   texDesc.height = 2;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   GRIS::StartRecordingCommandList( transferList );

   matTexture = GRIS::CreateTexture( transferList, texDesc, texData.data() );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   vertexCount = static_cast<uint32_t>( vertices.size() );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}

void RenderableComponent::uninit()
{
   GRIS::DestroyTexture( matTexture );
   GRIS::DestroyUniformBuffer( matBuffer );
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
}
}
