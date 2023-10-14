#include <ECS/Systems/Lighting/ShadowMapSystem.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>

#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Profiling.h>
#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t SHADOWMAP_DIM  = 2048;
static bool s_initialized                = false;
static PipelineIndex s_shadowmapPipeline = INVALID_PIPELINE_IDX;
static Framebuffer s_shadowmapFB         = {};

static void Initialize()
{
   s_shadowmapPipeline = StaticPipelines::FindByName( "TERRAIN_SHADOWMAP" );  // Hardcoded
   s_initialized       = true;
}

void ShadowMapSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "ShadowMapSystem" );

   if( !s_initialized )
   {
      Initialize();
   }

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Initialize shadow map texture and framebuffer
   if( !scene.shadowMap )
   {
      TextureDescription texDesc;
      texDesc.width  = SHADOWMAP_DIM;
      texDesc.height = SHADOWMAP_DIM;
      texDesc.type   = ImageType::TEXTURE_2D;
      texDesc.format = PixelFormat::D32_SFLOAT;
      texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::DEPTH_STENCIL;
      texDesc.stages = PipelineStage::FRAGMENT_STAGE;
      texDesc.name   = "Shadow Map";

      scene.shadowMap = GRIS::CreateTexture( texDesc );

      ClearValue clear;
      clear.depthStencil.depth   = 0.0f;
      clear.depthStencil.stencil = 0;

      s_shadowmapFB.resize( SHADOWMAP_DIM, SHADOWMAP_DIM );
      s_shadowmapFB.setToClearAll( true );
      s_shadowmapFB.attach( 0, scene.shadowMap, Access::DEPTH_STENCIL_ATTACHMENT_READ, clear );
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "ShadowMapSystem" );

   // TODO go through this loop once per light view
   GRIS::BeginRendering( cmdList, s_shadowmapFB );

   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   const PipelineInfo& pipInfo = *StaticPipelines::Get( s_shadowmapPipeline );
   GRIS::BindPipeline( cmdList, s_shadowmapPipeline );

   // Tracking
   std::string_view prevMesh;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );

      if( !renderable.isShadowCasting ) continue;

      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MaterialComponent& material   = *std::get<MaterialComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      const glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );

      GRIS::NamedUpdateConstantBuffer( cmdList, "Model", &modelMatrix, pipInfo );

      GRIS::NamedBufferBinding(
          cmdList, scene.viewsBuffer, "Views", pipInfo, 0, sizeof( scene.views ) );

      if( renderable.isInstanced )
      {
         CYD_ASSERT( renderable.instancesBuffer && "Invalid instance buffer" );
         GRIS::NamedBufferBinding( cmdList, renderable.instancesBuffer, "InstancesData", pipInfo );
      }

      if( renderable.isTessellated )
      {
         CYD_ASSERT( renderable.tessellationBuffer && "Invalid tessellation params buffer" );
         GRIS::NamedBufferBinding(
             cmdList, renderable.tessellationBuffer, "TessellationParams", pipInfo );
      }

      if( prevMaterial != material.materialIdx )
      {
         m_materials.bind( cmdList, material.materialIdx, 1 /*set*/ );
         prevMaterial = material.materialIdx;
      }

      if( prevMesh != mesh.asset )
      {
         if( mesh.vertexBuffer )
         {
            GRIS::BindVertexBuffer<Vertex>( cmdList, mesh.vertexBuffer );
            if( mesh.indexBuffer )
            {
               // This renderable has an index buffer, use it to draw
               GRIS::BindIndexBuffer<uint32_t>( cmdList, mesh.indexBuffer );
            }
         }

         prevMesh = mesh.asset;
      }

      if( renderable.instanceCount )
      {
         if( mesh.indexCount )
         {
            GRIS::DrawIndexedInstanced( cmdList, mesh.indexCount, renderable.instanceCount );
         }
         else
         {
            GRIS::DrawInstanced( cmdList, mesh.vertexCount, renderable.instanceCount );
         }
      }
      else
      {
         if( mesh.indexCount )
         {
            GRIS::DrawIndexed( cmdList, mesh.indexCount );
         }
         else
         {
            GRIS::Draw( cmdList, mesh.vertexCount );
         }
      }
   }

   GRIS::EndRendering( cmdList );
}
}