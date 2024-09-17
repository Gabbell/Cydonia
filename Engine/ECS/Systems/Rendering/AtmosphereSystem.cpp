#include <ECS/Systems/Rendering/AtmosphereSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/GRIS/TextureCache.h>

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
void AtmosphereSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::POST_PROCESS );
   CYD_SCOPED_GPUTRACE( cmdList, "AtmosphereSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   const uint32_t groupsX = static_cast<uint32_t>( std::ceil( scene.viewport.width / localSizeX ) );
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( scene.viewport.height / localSizeY ) );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      const AtmosphereComponent& atmos      = GetComponent<AtmosphereComponent>( entityEntry );

      if( !renderable.desc.isVisible )
      {
         continue;
      }

      // Output to color
      GRIS::BindPipeline( cmdList, s_atmosOutputPip );

      // We're using texture+samplers here instead of images to reduce banding
      GRIS::BindUniformBuffer( cmdList, scene.inverseViewsBuffer, 0 );
      GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 1 );

      GRIS::BindTexture( cmdList, scene.mainDepth, 2 );
      GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 3 );
      GRIS::BindTexture( cmdList, atmos.skyViewLUT, 4 );
      GRIS::BindTexture( cmdList, atmos.aerialPerspectiveLUT, 5 );

      if( scene.quarterResShadowMask )
      {
         GRIS::BindTexture( cmdList, scene.quarterResShadowMask, 6 );
      }
      else
      {
         GRIS::TextureCache::BindWhiteTexture( cmdList, 6, 0 );
      }

      GRIS::BindImage( cmdList, scene.mainColor, 7 );

      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
   }
}
}
