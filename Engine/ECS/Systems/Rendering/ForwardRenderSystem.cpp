#include <ECS/Systems/Rendering/ForwardRenderSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/ViewComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
// ================================================================================================
void ForwardRenderSystem::sort()
{
   auto forwardRenderSort = []( const EntityEntry& first, const EntityEntry& second )
   {
      const RenderableComponent& firstRenderable  = GetComponent<RenderableComponent>( first );
      const RenderableComponent& secondRenderable = GetComponent<RenderableComponent>( second );

      return ( firstRenderable.desc.type == RenderableComponent::Type::FORWARD ||
               secondRenderable.desc.type != RenderableComponent::Type::FORWARD ) &&
             first.handle < second.handle;
   };

   std::sort( m_entities.begin(), m_entities.end(), forwardRenderSort );
}

// ================================================================================================
void ForwardRenderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::OPAQUE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "ForwardRenderSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   GRIS::BeginRendering( cmdList, scene.mainFramebuffer );

   // Dynamic state
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   // Tracking
   MeshIndex prevMesh         = INVALID_MESH_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      const TransformComponent& transform   = GetComponent<TransformComponent>( entityEntry );
      const MaterialComponent& material     = GetComponent<MaterialComponent>( entityEntry );
      const MeshComponent& mesh             = GetComponent<MeshComponent>( entityEntry );

      if( renderable.desc.type != RenderableComponent::Type::FORWARD )
      {
         // Forward renderables only
         break;
      }

      if( !renderable.desc.isVisible )
      {
         continue;
      }

      if( renderable.pipelineIdx == INVALID_PIPELINE_IDX || mesh.meshIdx == INVALID_MESH_IDX ||
          material.materialIdx == INVALID_MATERIAL_IDX )
      {
         continue;
      }

      // Pipeline
      // ==========================================================================================
      const PipelineInfo* curPipInfo = nullptr;
      if( prevPipeline != renderable.pipelineIdx )
      {
         curPipInfo = StaticPipelines::Get( renderable.pipelineIdx );
         if( curPipInfo == nullptr )
         {
            // TODO WARNINGS
            printf( "ForwardRenderSystem: Passed a null pipeline, skipping entity\n" );
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

      // Optional Textures & Buffers
      // ==========================================================================================
      const glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );

      GRIS::NamedUpdateConstantBuffer( cmdList, "Model", &modelMatrix, *curPipInfo );

      GRIS::NamedBufferBinding(
          cmdList, scene.viewsBuffer, "Views", *curPipInfo, 0, sizeof( scene.views ) );

      GRIS::NamedBufferBinding( cmdList, scene.lightsBuffer, "Lights", *curPipInfo );

      if( renderable.desc.isShadowReceiving )
      {
         CYD_ASSERT( scene.shadowMap );

         SamplerInfo sampler;
         sampler.useCompare  = true;  // For PCF
         sampler.compare     = CompareOperator::GREATER_EQUAL;
         sampler.addressMode = AddressMode::CLAMP_TO_BORDER;
         sampler.borderColor = BorderColor::OPAQUE_BLACK;
         GRIS::NamedTextureBinding( cmdList, scene.shadowMap, sampler, "ShadowMap", *curPipInfo );
      }

      if( renderable.desc.useEnvironmentMap )
      {
         CYD_ASSERT( scene.envMap );
         GRIS::NamedTextureBinding( cmdList, scene.envMap, "EnvironmentMap", *curPipInfo );
      }

      if( renderable.isInstanced )
      {
         CYD_ASSERT( renderable.instancesBuffer && "Invalid instance buffer" );
         GRIS::NamedBufferBinding( cmdList, renderable.instancesBuffer, "Instances", *curPipInfo );
      }

      if( renderable.isTessellated )
      {
         CYD_ASSERT( renderable.tessellationBuffer && "Invalid tessellation params buffer" );
         GRIS::NamedBufferBinding(
             cmdList, renderable.tessellationBuffer, "TessellationParams", *curPipInfo );
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

   scene.mainFramebuffer.setClearAll( false );
}
}
