#include <ECS/Systems/Resources/MaterialLoaderSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include "Graphics/StaticPipelines.h"
#include "Graphics/Scene/MaterialCache.h"

#include <Profiling.h>

namespace CYD
{
void MaterialLoaderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "MaterialLoaderSystem" );

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::LOAD );

   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& material = *std::get<MaterialComponent*>( entityEntry.arch );

      if( !material.isLoaded )
      {
         material.pipelineIdx = StaticPipelines::FindByName( material.description.pipelineName );
         material.materialIdx = m_materials.getMaterialByName( material.description.materialName );

         CYD_ASSERT( material.pipelineIdx != INVALID_PIPELINE_IDX );
         CYD_ASSERT( material.materialIdx != INVALID_MATERIAL_IDX );

         m_materials.load( cmdList, material.materialIdx );
      }

      material.isLoaded = true;
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();
}
}
