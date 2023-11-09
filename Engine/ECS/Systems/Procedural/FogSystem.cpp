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

   const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "MAIN" );
   if( it == scene.viewNames.end() )
   {
      // TODO WARNING
      CYD_ASSERT( !"Could not find main view, skipping render tick" );
      return;
   }
   const uint32_t viewIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );
   const SceneComponent::ViewShaderParams& view   = scene.views[viewIdx];
   const SceneComponent::LightShaderParams& light = scene.lights[0];

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );
      if( !renderable.desc.isVisible ) continue;

      FogComponent& fog = *std::get<FogComponent*>( entityEntry.arch );

      if( !fog.viewInfoBuffer )
      {
         fog.viewInfoBuffer =
             GRIS::CreateUniformBuffer( sizeof( FogComponent::ViewInfo ), "Fog View Info" );
      }

      fog.viewInfo.invProj  = glm::inverse( view.projMat );  // Maybe we can avoid this?
      fog.viewInfo.invView  = glm::inverse( view.viewMat );
      fog.viewInfo.viewPos  = view.position;
      fog.viewInfo.lightDir = light.direction;

      fog.params.time += static_cast<float>( deltaS );

      GRIS::BindPipeline( cmdList, s_fogPipeline );

      const UploadToBufferInfo info = { 0, sizeof( FogComponent::ViewInfo ) };
      GRIS::UploadToBuffer( fog.viewInfoBuffer, &fog.viewInfo, info );

      GRIS::BindUniformBuffer( cmdList, fog.viewInfoBuffer, 0 );
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