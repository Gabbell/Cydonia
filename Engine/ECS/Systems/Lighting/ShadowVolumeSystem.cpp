#include <ECS/Systems/Lighting/ShadowVolumeSystem.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>
#include <Graphics/Utility/ShadowMapping.h>

#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
static bool s_initialized            = false;
static PipelineIndex s_shadowMaskPip = INVALID_PIPELINE_IDX;
static PipelineIndex s_blurPip       = INVALID_PIPELINE_IDX;
static Framebuffer s_shadowMaskFB    = {};
static TextureHandle s_pingPongTex   = {};  // TODO Part of texture cache
static Framebuffer s_blurFB          = {};  // Used as ping pong framebuffer

static void Initialize()
{
   s_shadowMaskPip = StaticPipelines::FindByName( "TERRAIN_SHADOWMASK" );  // TODO
   s_blurPip       = StaticPipelines::FindByName( "GAUSSIAN_BLUR" );
   s_initialized   = true;
}

// Gaussian blur the mask to smooth out noise
// This could be optimized in the following way:
// * Have a texture cache for the ping pong texture
// * Not swap render targets for the two direction
static void GaussianBlur( CmdListHandle cmdList, const SceneComponent& scene )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Gaussian Blur Pass" );

   // Horizontal pass
   uint32_t isHorizontal = 1;

   GRIS::BeginRendering( cmdList, s_blurFB );
   GRIS::BindPipeline( cmdList, s_blurPip );
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );
   GRIS::BindTexture( cmdList, scene.quarterResShadowMask, 0 );
   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::FRAGMENT_STAGE, 0, sizeof( isHorizontal ), &isHorizontal );
   GRIS::Draw( cmdList, 3, 0 );
   GRIS::EndRendering( cmdList );

   // Vertical pass
   isHorizontal = 0;

   GRIS::BeginRendering( cmdList, s_shadowMaskFB );
   GRIS::BindPipeline( cmdList, s_blurPip );
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );
   GRIS::BindTexture( cmdList, s_pingPongTex, 0 );
   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::FRAGMENT_STAGE, 0, sizeof( isHorizontal ), &isHorizontal );
   GRIS::Draw( cmdList, 3, 0 );
   GRIS::EndRendering( cmdList );
}

// ================================================================================================
void ShadowVolumeSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER_P2 );
   CYD_SCOPED_GPUTRACE( cmdList, "ShadowVolumeSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Initialize shadow map texture and framebuffer
   if( scene.resolutionChanged )
   {
      GRIS::DestroyTexture( scene.quarterResShadowMask );
      scene.quarterResShadowMask = {};
   }

   if( !scene.quarterResShadowMask )
   {
      const uint32_t quarterWidth  = scene.extent.width / 4;
      const uint32_t quarterHeight = scene.extent.height / 4;

      TextureDescription texDesc;
      texDesc.width  = quarterWidth;
      texDesc.height = quarterHeight;
      texDesc.type   = ImageType::TEXTURE_2D;
      texDesc.format = PixelFormat::R32F;
      texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::COLOR;
      texDesc.stages = PipelineStage::FRAGMENT_STAGE;
      texDesc.name   = "Shadow Mask Quarter Res";

      scene.quarterResShadowMask = GRIS::CreateTexture( texDesc );

      texDesc.name = "Shadow Mask Ping Pong";

      s_pingPongTex = GRIS::CreateTexture( texDesc );

      // We clear in case we don't write anything in the framebuffer
      ClearValue colorClear;
      colorClear.color.f32[0] = 1.0f;
      colorClear.color.f32[1] = 0.0f;
      colorClear.color.f32[2] = 0.0f;
      colorClear.color.f32[3] = 0.0f;

      s_shadowMaskFB.resize( quarterWidth, quarterHeight );
      s_shadowMaskFB.replace(
          0, scene.quarterResShadowMask, Access::FRAGMENT_SHADER_READ, colorClear );
      s_shadowMaskFB.setClearAll( true );

      s_blurFB.resize( quarterWidth, quarterHeight );
      s_blurFB.replace( 0, s_pingPongTex, Access::FRAGMENT_SHADER_READ );
   }

   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   GRIS::BeginRendering( cmdList, s_shadowMaskFB );

   GRIS::BindPipeline( cmdList, s_shadowMaskPip );

   // Dynamic state
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   GRIS::BindUniformBuffer( cmdList, scene.viewsBuffer, 0 );
   GRIS::BindUniformBuffer( cmdList, scene.inverseViewsBuffer, 1 );
   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 2 );
   GRIS::BindUniformBuffer( cmdList, scene.shadowMapsBuffer, 3 );

   if( scene.shadowMapTextures[0] )
   {
      GRIS::BindTexture( cmdList, scene.shadowMapTextures[0], ShadowMapping::GetSampler(), 4 );
   }

   GRIS::BindTexture( cmdList, scene.mainDepth, 5 );

   // Iterate through entities
   bool populated = false;
   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      const TransformComponent& transform   = GetComponent<TransformComponent>( entityEntry );
      const MaterialComponent& material     = GetComponent<MaterialComponent>( entityEntry );

      if( !renderable.desc.isVisible || !renderable.desc.isVolumeShadowCasting ||
          renderable.pipelineIdx == INVALID_PIPELINE_IDX ||
          material.materialIdx == INVALID_MATERIAL_IDX )
      {
         continue;
      }

      populated = true;

      glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );
      modelMatrix = glm::inverse( modelMatrix );

      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::FRAGMENT_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      // Material
      // ==========================================================================================
      if( prevMaterial != material.materialIdx )
      {
         // We need the displacement map to raymarch it
         m_materials.bindSlot(
             cmdList,
             material.materialIdx,
             MaterialCache::TextureSlot::DISPLACEMENT,
             6,
             0 /*set*/ );

         prevMaterial = material.materialIdx;
      }

      GRIS::Draw( cmdList, 3, 0 );
   }

   GRIS::EndRendering( cmdList );

   if( populated )
   {
      // If we drew into the shadow mask, blur it
      GaussianBlur( cmdList, scene );
   }
}
}
