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
static bool s_initialized = false;

enum DeferredPipelines
{
   BLINN_PHONG,
   PBR,
   COUNT
};

static PipelineIndex s_pipelines[DeferredPipelines::COUNT] = {};
static Framebuffer s_deferredFB                            = {};

void DeferredRenderSystem::sort()
{
   auto deferredRenderSort = []( const EntityEntry& first, const EntityEntry& second )
   {
      const RenderableComponent& firstRenderable  = GetComponent<RenderableComponent>( first );
      const RenderableComponent& secondRenderable = GetComponent<RenderableComponent>( second );

      const bool firstIsDeferred = firstRenderable.desc.type == RenderableComponent::Type::DEFERRED;
      const bool secondIsDeferred =
          secondRenderable.desc.type == RenderableComponent::Type::DEFERRED;

      return firstIsDeferred && !secondIsDeferred;
   };

   std::sort( m_entities.begin(), m_entities.end(), deferredRenderSort );
}

static void Initialize()
{
   s_pipelines[BLINN_PHONG] = StaticPipelines::FindByName( "BLINN_PHONG_DEFERRED" );
   s_pipelines[PBR]         = StaticPipelines::FindByName( "PBR_DEFERRED" );
   s_initialized            = true;
}

// ================================================================================================
void DeferredRenderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

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
      s_deferredFB.resize( scene.extent.width, scene.extent.height );
      s_deferredFB.replace( Framebuffer::COLOR, scene.mainColor, Access::COLOR_ATTACHMENT_WRITE );
   }

   // Determine if we have any deferred renderables
   bool shouldRender = false;
   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      if( renderable.desc.type == RenderableComponent::Type::DEFERRED )
      {
         shouldRender = true;
         break;
      }
   }

   // We use a separate framebuffer than the main one because we are sampling depth, instead of
   // writing into it
   GRIS::BeginRendering( cmdList, s_deferredFB );

   if( shouldRender )
   {
      // Dynamic state
      GRIS::SetViewport( cmdList, {} );
      GRIS::SetScissor( cmdList, {} );

      GRIS::BindPipeline( cmdList, s_pipelines[PBR] );

      GRIS::BindUniformBuffer( cmdList, scene.viewsBuffer, 0, 0, 0, sizeof( scene.views ) );
      GRIS::BindUniformBuffer(
          cmdList, scene.inverseViewsBuffer, 1, 0, 0, sizeof( scene.inverseViews ) );
      GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 2, 0, 0, sizeof( scene.lights ) );

      scene.gbuffer.bindTexture( cmdList, GBuffer::ALBEDO, 0, 1 );
      scene.gbuffer.bindTexture( cmdList, GBuffer::NORMAL, 1, 1 );
      scene.gbuffer.bindTexture( cmdList, GBuffer::PBR, 2, 1 );
      scene.gbuffer.bindTexture( cmdList, GBuffer::DEPTH, 3, 1 );
      scene.gbuffer.bindTexture( cmdList, GBuffer::SHADOW, 4, 1 );

      GRIS::Draw( cmdList, 3, 0 );
   }

   GRIS::EndRendering( cmdList );

   // Consume the clear because we've effectively cleared the main color using this pass
   scene.mainFramebuffer.setClearAll( false );
}
}
