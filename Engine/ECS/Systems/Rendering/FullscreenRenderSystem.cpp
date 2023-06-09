#include <ECS/Systems/Rendering/FullscreenRenderSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

namespace CYD
{
// ================================================================================================
void FullscreenRenderSystem::tick( double /*deltaS*/ )
{
   // Start command list recording
   const CmdListHandle cmdList = GRIS::GetMainCommandList();

   // Dynamic state
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   // Rendering straight to swapchain
   GRIS::BeginRendering( cmdList );

   static PipelineIndex pipelineIdx = INVALID_PIPELINE_IDX;
   if( pipelineIdx == INVALID_PIPELINE_IDX )
   {
      pipelineIdx = StaticPipelines::FindByName( "FULLSCREEN_2DTEX" );
   }

   // Tracking
   GRIS::BindPipeline( cmdList, pipelineIdx );

   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      const StaticMaterialComponent& material =
          *std::get<StaticMaterialComponent*>( entityEntry.arch );

      // Material
      if( prevMaterial != material.materialIdx )
      {
         m_materials.bind( cmdList, material.materialIdx, 0 /*set*/ );
         prevMaterial = material.materialIdx;
      }

      // Draw fullscreen quad without vertex buffer
      GRIS::Draw( cmdList, 3, 0 );
   }

   GRIS::EndRendering( cmdList );

   RenderGraph::AddPass( cmdList );
}
}