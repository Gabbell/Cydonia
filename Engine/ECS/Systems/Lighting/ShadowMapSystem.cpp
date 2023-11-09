#include <ECS/Systems/Lighting/ShadowMapSystem.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

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
   CYD_TRACE();

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
      s_shadowmapFB.setClearAll( true );
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
   MeshIndex prevMesh         = INVALID_MESH_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;

   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );

      if( !renderable.desc.isShadowCasting )
      {
         continue;
      }

      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MaterialComponent& material   = *std::get<MaterialComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      // Vertex and index buffers
      // ==========================================================================================
      if( prevMesh != mesh.meshIdx )
      {
         if( mesh.meshIdx == INVALID_MESH_IDX )
         {
            continue;
         }

         m_meshes.bind( cmdList, mesh.meshIdx );

         prevMesh = mesh.meshIdx;
      }

      // Material
      // ==========================================================================================
      if( prevMaterial != material.materialIdx )
      {
         if( material.materialIdx == INVALID_MATERIAL_IDX )
         {
            continue;
         }

         m_materials.bind( cmdList, material.materialIdx, 1 /*set*/ );

         prevMaterial = material.materialIdx;
      }

      // Optional Buffers
      // ==========================================================================================
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