#include <ECS/Systems/Rendering/GBufferSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/TextureCache.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>
#include <Graphics/Utility/ShadowMapping.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/ViewComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/Vulkan/Synchronization.h>

#include <Profiling.h>

namespace CYD
{
// ================================================================================================
void GBufferSystem::sort()
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

// ================================================================================================
void GBufferSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER_P2 );
   CYD_SCOPED_GPUTRACE( cmdList, "GBufferSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Initialize/reset GBuffer
   if( scene.resolutionChanged )
   {
      ClearValue clearVal;
      clearVal.depthStencil.depth   = 1.0f;
      clearVal.depthStencil.stencil = 0;

      scene.gbuffer.resize( scene.extent.width, scene.extent.height );
      scene.gbuffer.replace(
          GBuffer::DEPTH, scene.mainDepth, Access::DEPTH_STENCIL_ATTACHMENT_READ, clearVal );
      scene.gbuffer.setClearAll( true );
   }

   // Tracking
   MeshIndex prevMesh         = INVALID_MESH_IDX;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   // Rendering to the gbuffer
   GRIS::BeginRendering( cmdList, scene.gbuffer );

   // Dynamic state
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      const TransformComponent& transform   = GetComponent<TransformComponent>( entityEntry );
      const MaterialComponent& material     = GetComponent<MaterialComponent>( entityEntry );
      const MeshComponent& mesh             = GetComponent<MeshComponent>( entityEntry );

      if( renderable.desc.type != RenderableComponent::Type::DEFERRED )
      {
         // Deferred renderables only
         break;
      }

      if( !renderable.desc.isVisible || renderable.pipelineIdx == INVALID_PIPELINE_IDX ||
          mesh.meshIdx == INVALID_MESH_IDX )
      {
         continue;
      }

      CYD_SCOPED_GPUTRACE( cmdList, m_ecs->getEntity( entityEntry.handle )->getName().c_str() );

      // Pipeline
      // ==========================================================================================
      const PipelineInfo* curPipInfo = nullptr;
      if( prevPipeline != renderable.pipelineIdx )
      {
         curPipInfo = StaticPipelines::Get( renderable.pipelineIdx );
         if( curPipInfo == nullptr )
         {
            // TODO WARNINGS
            printf( "GBufferSystem: Passed a null pipeline, skipping entity\n" );
            continue;
         }

         GRIS::BindPipeline( cmdList, curPipInfo );

         prevPipeline = renderable.pipelineIdx;
      }

      // Vertex and index buffers
      // ==========================================================================================
      if( prevMesh != mesh.meshIdx )
      {
         m_meshes.bind( cmdList, mesh.meshIdx );

         prevMesh = mesh.meshIdx;
      }

      // Material
      // ==========================================================================================
      if( prevMaterial != material.materialIdx )
      {
         m_materials.bind( cmdList, material.materialIdx, 1 /*set*/ );

         prevMaterial = material.materialIdx;
      }

      // Optional Buffers
      // ==========================================================================================
      const glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );

      GRIS::NamedUpdateConstantBuffer( cmdList, "MODEL", &modelMatrix, *curPipInfo );
      GRIS::NamedBufferBinding( cmdList, scene.viewsBuffer, "VIEWS", *curPipInfo );
      // GRIS::NamedBufferBinding( cmdList, scene.frustumsBuffer, "FRUSTUMS", *curPipInfo );
      GRIS::NamedBufferBinding( cmdList, scene.lightsBuffer, "LIGHTS", *curPipInfo );
      GRIS::NamedBufferBinding( cmdList, scene.shadowMapsBuffer, "SHADOWS", *curPipInfo );

     if( renderable.desc.isShadowReceiving )
      {
         if( scene.shadowMapTextures[0] )
         {
            GRIS::NamedTextureBinding(
                cmdList,
                scene.shadowMapTextures[0],
                ShadowMapping::GetSampler(),
                "SHADOWMAP",
                *curPipInfo );
         }
         else
         {
            GRIS::NamedTextureBinding(
                cmdList,
                GRIS::TextureCache::GetDepthTextureArray(),
                ShadowMapping::GetSampler(),
                "SHADOWMAP",
                *curPipInfo );
         }
      }

      if( renderable.isInstanced )
      {
         CYD_ASSERT( renderable.instancesBuffer && "Invalid instance buffer" );
         GRIS::NamedBufferBinding( cmdList, renderable.instancesBuffer, "INSTANCES", *curPipInfo );
      }

      if( renderable.isTessellated )
      {
         CYD_ASSERT( renderable.tessellationBuffer && "Invalid tessellation buffer" );
         GRIS::NamedBufferBinding(
             cmdList, renderable.tessellationBuffer, "TESSELLATION", *curPipInfo );
      }

      // Draw
      // ==========================================================================================
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
