#include <ECS/Systems/Resources/MeshLoaderSystem.h>

#include <Graphics/AssetStash.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
static bool findMesh( MeshComponent& mesh, AssetStash& assets )
{
   // Check if the mesh is already in the asset stash
   const Mesh& loadedMesh = assets.getMesh( mesh.asset );
   if( loadedMesh.vertexCount )
   {
      mesh.vertexBuffer = loadedMesh.vertexBuffer;
      mesh.indexBuffer  = loadedMesh.indexBuffer;
      mesh.vertexCount  = loadedMesh.vertexCount;
      mesh.indexCount   = loadedMesh.indexCount;

      return true;
   }

   return false;
}

void MeshLoaderSystem::tick( double /*deltaS*/ )
{
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER, "MeshLoaderSystem" );

   GRIS::StartRecordingCommandList( transferList );

   for( const auto& entityEntry : m_entities )
   {
      MeshComponent& mesh = *std::get<MeshComponent*>( entityEntry.arch );

      if( !mesh.asset.empty() && !findMesh( mesh, m_assets ) )
      {
         m_assets.loadMeshFromPath( transferList, mesh.asset );

         const bool foundMesh = findMesh( mesh, m_assets );
         CYDASSERT( foundMesh && "MeshLoaderSystem: A named mesh could not be loaded" );
      }
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();

   GRIS::EndRecordingCommandList( transferList );

   RenderGraph::AddPass( transferList );
}
}
