#include <ECS/Systems/Rendering/GBufferSystem.h>

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

#include <Graphics/Vulkan/Synchronization.h>

#include <Profiling.h>

namespace CYD
{
// ================================================================================================
void GBufferSystem::sort()
{
   auto deferredRenderSort = []( const EntityEntry& first, const EntityEntry& second )
   {
      const RenderableComponent& firstRenderable  = *std::get<RenderableComponent*>( first.arch );
      const RenderableComponent& secondRenderable = *std::get<RenderableComponent*>( second.arch );

      return firstRenderable.type == RenderableComponent::Type::DEFERRED ||
             secondRenderable.type != RenderableComponent::Type::DEFERRED;
   };

   std::sort( m_entities.begin(), m_entities.end(), deferredRenderSort );
}

// ================================================================================================
void GBufferSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "GBufferSystem" );

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "GBufferSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Initialize/reset GBuffer
   if( scene.resolutionChanged )
   {
      scene.gbuffer.setToClearAll( true );
      scene.gbuffer.resize( scene.extent.width, scene.extent.height );
      scene.gbuffer.replace(
          GBuffer::DEPTH, scene.mainDepth, Access::DEPTH_STENCIL_ATTACHMENT_READ );
   }

   // Finding main view
   const uint32_t mainViewIdx = getViewIndex( scene, "MAIN" );
   const uint32_t sunViewIdx  = getViewIndex( scene, "SUN" );

   // Tracking
   std::string_view prevMesh;
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
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );
      if( renderable.type != RenderableComponent::Type::DEFERRED )
      {
         // Deferred renderables only
         break;
      }

      if( !renderable.isVisible ) continue;

      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MaterialComponent& material   = *std::get<MaterialComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      // Pipeline
      // ==========================================================================================
      const PipelineInfo* curPipInfo = nullptr;
      if( prevPipeline != material.pipelineIdx )
      {
         curPipInfo = StaticPipelines::Get( material.pipelineIdx );
         if( curPipInfo == nullptr )
         {
            // TODO WARNINGS
            printf( "GBufferSystem: Passed a null pipeline, skipping entity\n" );
            continue;
         }

         GRIS::BindPipeline( cmdList, curPipInfo );

         prevPipeline = material.pipelineIdx;
      }

      // Optional Buffers
      // ==========================================================================================
      GRIS::NamedBufferBinding(
          cmdList, scene.viewsBuffer, "Views", *curPipInfo, 0, sizeof( scene.views ) );

      if( renderable.isShadowReceiving && scene.shadowMap )
      {
         SamplerInfo sampler;
         sampler.useCompare  = true;  // For PCF
         sampler.compare     = CompareOperator::GREATER_EQUAL;
         sampler.addressMode = AddressMode::CLAMP_TO_BORDER;
         sampler.borderColor = BorderColor::OPAQUE_BLACK;
         GRIS::BindTexture( cmdList, scene.shadowMap, sampler, 1, 1 );
      }

      if( renderable.isInstanced )
      {
         CYD_ASSERT( renderable.instancesBuffer && "Invalid instance buffer" );
         GRIS::NamedBufferBinding(
             cmdList, renderable.instancesBuffer, "InstancesData", *curPipInfo );
      }

      if( renderable.isTessellated )
      {
         CYD_ASSERT( renderable.tessellationBuffer && "Invalid tessellation params buffer" );
         GRIS::NamedBufferBinding(
             cmdList, renderable.tessellationBuffer, "TessellationParams", *curPipInfo );
      }

      // Push Constants
      // ==========================================================================================
      const glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );

      GRIS::NamedUpdateConstantBuffer( cmdList, "Model", &modelMatrix, *curPipInfo );

      // Material
      // ==========================================================================================
      if( material.materialIdx != INVALID_MATERIAL_IDX )
      {
         m_materials.bind( cmdList, material.materialIdx, 1 /*set*/ );
         prevMaterial = material.materialIdx;
      }

      // Vertex and index buffers
      // ==========================================================================================
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
