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
   CYDTRACE( "MaterialLoaderSystem" );

   const CmdListHandle cmdList = GRIS::GetMainCommandList();

   for( const auto& entityEntry : m_entities )
   {
      StaticMaterialComponent& material = *std::get<StaticMaterialComponent*>( entityEntry.arch );

      if( !material.isLoaded )
      {
         material.pipelineIdx = StaticPipelines::FindByName( material.pipelineName );
         material.materialIdx = m_materials.getMaterialByName( material.materialName );

         CYDASSERT( material.pipelineIdx != INVALID_PIPELINE_IDX );
         CYDASSERT( material.materialIdx != INVALID_MATERIAL_IDX );

         m_materials.load( cmdList, material.materialIdx );
      }

      material.isLoaded = true;
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();
}
}
