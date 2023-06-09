#include <ECS/Systems/Resources/MeshLoaderSystem.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <Graphics/Scene/MeshCache.h>

#include <Profiling.h>

namespace CYD
{
static bool findMesh( MeshComponent& mesh, MeshCache& cache )
{
   // Check if the mesh is already in the asset stash
   const Mesh& loadedMesh = cache.getMesh( mesh.asset );
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
   CYDTRACE( "MeshLoaderSystem" );
   
   const CmdListHandle cmdList = GRIS::GetMainCommandList();

   for( const auto& entityEntry : m_entities )
   {
      MeshComponent& mesh = *std::get<MeshComponent*>( entityEntry.arch );

      if( !mesh.asset.empty() && !findMesh( mesh, m_meshCache ) )
      {
         m_meshCache.loadMeshFromPath( cmdList, mesh.asset );

         const bool foundMesh = findMesh( mesh, m_meshCache );
         CYDASSERT( foundMesh && "MeshLoaderSystem: A named mesh could not be loaded" );
      }
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();
}
}
