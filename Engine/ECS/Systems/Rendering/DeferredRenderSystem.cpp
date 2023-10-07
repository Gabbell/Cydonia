#include <ECS/Systems/Rendering/DeferredRenderSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/ViewComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
static bool s_initialized               = false;
static PipelineIndex s_lightingPipeline = INVALID_PIPELINE_IDX;
static Framebuffer s_lightingFB         = {};

static void Initialize()
{
   s_lightingPipeline = StaticPipelines::FindByName( "DEFERRED_LIGHTING" );
   s_initialized      = true;
}

// ================================================================================================
void DeferredRenderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "DeferredRenderSystem" );

   if( !s_initialized )
   {
      Initialize();
   }

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::OPAQUE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "DeferredRenderSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   if( scene.resolutionChanged )
   {
      s_lightingFB.resize( scene.extent.width, scene.extent.height );
      s_lightingFB.replace( Framebuffer::COLOR, scene.mainColor, Access::COLOR_ATTACHMENT_WRITE );
      s_lightingFB.setToClear( Framebuffer::COLOR, true );
   }

   GRIS::BeginRendering( cmdList, s_lightingFB );

   // Dynamic state
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   GRIS::BindPipeline( cmdList, s_lightingPipeline );

   GRIS::BindUniformBuffer( cmdList, scene.viewsBuffer, 4, 0, 0, sizeof( scene.views ) );
   GRIS::BindUniformBuffer(
       cmdList, scene.inverseViewsBuffer, 5, 0, 0, sizeof( scene.inverseViews ) );
   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 6, 0, 0, sizeof( scene.lights ) );

   scene.gbuffer.bind( cmdList );

   GRIS::Draw( cmdList, 3, 0 );

   GRIS::EndRendering( cmdList );
}
}
