#include <ECS/Systems/Rendering/AtmosphereRenderSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/ViewComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/Vulkan/Synchronization.h>

#include <Profiling.h>

namespace CYD
{
static bool s_initialized             = false;
static PipelineIndex s_atmosOutputPip = INVALID_PIPELINE_IDX;

static void Initialize()
{
   s_atmosOutputPip = StaticPipelines::FindByName( "ATMOS_OUTPUT" );
   s_initialized    = true;
}

// ================================================================================================
void AtmosphereRenderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::POST_PROCESS );
   CYD_SCOPED_GPUTRACE( cmdList, "AtmosphereRenderSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   CYD_GPUTRACE_BEGIN( cmdList, "Atmosphere Output" );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX = static_cast<uint32_t>( std::ceil( scene.viewport.width / localSizeX ) );
   uint32_t groupsY = static_cast<uint32_t>( std::ceil( scene.viewport.height / localSizeY ) );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const AtmosphereComponent& atmos = *std::get<AtmosphereComponent*>( entityEntry.arch );

      // Output to color
      GRIS::BindPipeline( cmdList, s_atmosOutputPip );

      // We're using texture+samplers here instead of images to reduce banding
      GRIS::BindUniformBuffer( cmdList, atmos.viewInfoBuffer, 0 );
      GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 1 );
      GRIS::BindTexture( cmdList, atmos.skyViewLUT, 2 );
      GRIS::BindTexture( cmdList, atmos.aerialPerspectiveLUT, 3 );
      GRIS::BindImage( cmdList, scene.mainColor, 4 );
      GRIS::BindTexture( cmdList, scene.mainDepth, 5 );

      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
   }

   CYD_GPUTRACE_END( cmdList );
}
}
