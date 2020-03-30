#include <ECS/Components/PhongRenderableComponent.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/GraphicsIO.h>

namespace cyd
{
// Placeholder texture
static constexpr uint32_t placeholderData[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF};
static constexpr TextureDescription placeholderDesc = {
    sizeof( placeholderData ),
    2,  // width
    2,  // height
    ImageType::TEXTURE_2D,
    PixelFormat::BGRA8_UNORM,
    ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED};

PhongRenderableComponent::PhongRenderableComponent( const std::vector<Vertex>& vertices )
    : RenderableComponent( RenderableType::PHONG )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   texture = GRIS::CreateTexture( transferList, placeholderDesc, placeholderData );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   vertexCount = static_cast<uint32_t>( vertices.size() );
   indexCount  = 0;

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}

PhongRenderableComponent::PhongRenderableComponent( const std::string& meshPath )
    : RenderableComponent( RenderableType::PHONG )
{
   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   GraphicsIO::LoadMesh( meshPath, vertices, indices );

   // Uploading data to GPU
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

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
   GRIS::DestroyCommandList( transferList );
}

void PhongRenderableComponent::uninit()
{
   GRIS::DestroyTexture( texture );
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
}
}
