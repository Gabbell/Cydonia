#include <ECS/Systems/Procedural/FogSystem.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
static bool s_initialized          = false;
static PipelineIndex s_fogPipeline = INVALID_PIPELINE_IDX;

static void Initialize()
{
   s_fogPipeline = StaticPipelines::FindByName( "FOG_COMPUTE" );

   s_initialized = true;
}

// ================================================================================================
void FogSystem::tick( double deltaS )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::POST_PROCESS );
   CYD_SCOPED_GPUTRACE( cmdList, "FogSystem" );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      FogComponent& fog                     = GetComponent<FogComponent>( entityEntry );

      if( !renderable.desc.isVisible )
      {
         continue;
      }

      fog.params.time += static_cast<float>( deltaS );

      GRIS::BindPipeline( cmdList, s_fogPipeline );

      GRIS::BindUniformBuffer( cmdList, scene.inverseViewsBuffer, 0 );
      GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 1 );
      GRIS::BindImage( cmdList, scene.mainColor, 1 );
      GRIS::BindTexture( cmdList, scene.mainDepth, 2 );

      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( fog.params ), &fog.params );

      constexpr float localSizeX = 16.0f;
      constexpr float localSizeY = 16.0f;
      uint32_t groupsX = static_cast<uint32_t>( std::ceil( scene.viewport.width / localSizeX ) );
      uint32_t groupsY = static_cast<uint32_t>( std::ceil( scene.viewport.height / localSizeY ) );

      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
   }
}
}