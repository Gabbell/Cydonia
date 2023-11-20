#include <ECS/Systems/Resources/PipelineLoaderSystem.h>

#include <Graphics/StaticPipelines.h>

#include <Profiling.h>

namespace CYD
{
void PipelineLoaderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   bool allLoaded = true;
   for( const auto& entityEntry : m_entities )
   {
      RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );

      if( renderable.desc.pipelineName.empty() )
      {
         continue;
      }

      const PipelineIndex pipelineIdx = StaticPipelines::FindByName( renderable.desc.pipelineName );
      if( pipelineIdx == INVALID_PIPELINE_IDX )
      {
         CYD_ASSERT( !"Could not find pipeline" );
         continue;
      }

      renderable.pipelineIdx = pipelineIdx;

      allLoaded &= ( pipelineIdx != INVALID_PIPELINE_IDX );
   }

   // We don't want to spend more time on loading these entities' resources
   if( allLoaded )
   {
      m_entities.clear();
   }
}
}
