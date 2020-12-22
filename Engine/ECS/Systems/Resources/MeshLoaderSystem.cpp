#include <ECS/Systems/Resources/MeshLoaderSystem.h>

#include <Graphics/AssetStash.h>
#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void MeshLoaderSystem::tick( double /*deltaS*/ )
{
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   for( const auto& entityEntry : m_entities )
   {
      MeshComponent& mesh = *std::get<MeshComponent*>( entityEntry.arch );

      const Mesh& loadedMesh = m_assets.loadMesh( transferList, mesh.asset );

      mesh.vertexBuffer = loadedMesh.vertexBuffer;
      mesh.indexBuffer  = loadedMesh.indexBuffer;
      mesh.vertexCount  = loadedMesh.vertexCount;
      mesh.indexCount   = loadedMesh.indexCount;
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}
}
