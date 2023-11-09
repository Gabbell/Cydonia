#include <ECS/Systems/Resources/MaterialLoaderSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/Utility/GraphicsIO.h>

#include <Profiling.h>

namespace CYD
{
void MaterialLoaderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::LOAD );

   bool allLoaded = true;
   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& materialComponent = *std::get<MaterialComponent*>( entityEntry.arch );

      const MaterialIndex materialIdx =
          m_materialCache.getMaterialIndexByName( materialComponent.materialName );

      if( materialIdx == INVALID_MATERIAL_IDX )
      {
         CYD_ASSERT( !"Could not find material" );
         continue;
      }

      const MaterialCache::State assetState = m_materialCache.progressLoad( cmdList, materialIdx );
      if( assetState == MaterialCache::State::LOADED_TO_VRAM )
      {
         materialComponent.materialIdx = materialIdx;

         CYD_ASSERT(
             materialComponent.materialIdx != INVALID_MATERIAL_IDX && "Could not find material" );
      }

      allLoaded &= ( assetState == MaterialCache::State::LOADED_TO_VRAM );
   }

   // We don't want to spend more time on loading these entities' resources
   if( allLoaded )
   {
      m_entities.clear();
   }
}
}
