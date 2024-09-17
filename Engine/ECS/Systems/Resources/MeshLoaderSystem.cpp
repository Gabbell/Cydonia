#include <ECS/Systems/Resources/MeshLoaderSystem.h>

#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <Graphics/Scene/MeshCache.h>

#include <Multithreading/ThreadPool.h>

#include <Profiling.h>

namespace CYD
{
void MeshLoaderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::LOAD );

   bool allLoaded = true;
   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      MeshComponent& meshComponent          = GetComponent<MeshComponent>( entityEntry );

      MeshIndex meshIdx = m_meshCache.findMesh( meshComponent.name );
      if( meshIdx == INVALID_MESH_IDX )
      {
         // Getting vertex layout from pipeline
         const PipelineInfo* pipInfo = StaticPipelines::Get( renderable.pipelineIdx );
         CYD_ASSERT( pipInfo && pipInfo->type == PipelineType::GRAPHICS );
         const GraphicsPipelineInfo* graphicsPip =
             static_cast<const GraphicsPipelineInfo*>( pipInfo );

         meshIdx = m_meshCache.addMesh( meshComponent.name, graphicsPip->vertLayout );
      }

      const MeshCache::State assetState = m_meshCache.progressLoad( cmdList, meshIdx );

      if( assetState == MeshCache::State::LOADED_TO_VRAM )
      {
         const MeshCache::DrawInfo drawInfo = m_meshCache.getDrawInfo( meshIdx );

         meshComponent.meshIdx     = meshIdx;
         meshComponent.vertexCount = drawInfo.vertexCount;
         meshComponent.indexCount  = drawInfo.indexCount;

         CYD_ASSERT( meshComponent.meshIdx != INVALID_MESH_IDX && "Could not find mesh" );
      }

      allLoaded &= ( assetState == MeshCache::State::LOADED_TO_VRAM );
   }

   // We don't want to spend more time on loading these entities' resources
   if( allLoaded )
   {
      m_entities.clear();
   }
}
}
