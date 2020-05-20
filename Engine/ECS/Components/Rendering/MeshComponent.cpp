#include <ECS/Components/Rendering/MeshComponent.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/GraphicsIO.h>

namespace CYD
{
bool MeshComponent::init( const std::vector<Vertex>& vertices )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

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

   return true;
}

bool MeshComponent::init( const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices )
{
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
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

bool MeshComponent::init( const std::string& meshPath )
{
   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   GraphicsIO::LoadMesh( meshPath, vertices, indices );

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
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

void MeshComponent::uninit()
{
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
}

MeshComponent::~MeshComponent() { MeshComponent::uninit(); }
}
