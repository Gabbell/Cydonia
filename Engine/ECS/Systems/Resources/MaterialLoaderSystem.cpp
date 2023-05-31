#include <ECS/Systems/Resources/MaterialLoaderSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include "Graphics/StaticPipelines.h"
#include "Graphics/Scene/MaterialCache.h"

namespace CYD
{
void MaterialLoaderSystem::tick( double /*deltaS*/ )
{
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER, "MaterialLoaderSystem" );

   GRIS::StartRecordingCommandList( transferList );

   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& material = *std::get<MaterialComponent*>( entityEntry.arch );

      if( !material.isLoaded )
      {
         material.pipelineIdx = StaticPipelines::FindByName( material.pipelineName );
         material.materialIdx = m_materials.getMaterialByName( material.materialName );

         m_materials.load( transferList, material.materialIdx );
      }

      CYDASSERT( material.pipelineIdx != INVALID_PIPELINE_IDX );
      CYDASSERT( material.materialIdx != INVALID_MATERIAL_IDX );

      material.isLoaded = true;
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();

   GRIS::EndRecordingCommandList( transferList );

   RenderGraph::AddPass( transferList );
}
}
