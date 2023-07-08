#include <ECS/Systems/Rendering/ForwardRenderSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/Utility/Transforms.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/CameraComponent.h>
#include <ECS/Components/Transforms/InstancedComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
// ================================================================================================
bool ForwardRenderSystem::_compareEntities( const EntityEntry& first, const EntityEntry& second )
{
   // We are sorting entities by their material/pipeline
   const MaterialComponent& matFirst  = *std::get<MaterialComponent*>( first.arch );
   const MaterialComponent& matSecond = *std::get<MaterialComponent*>( second.arch );

   return matFirst.materialIdx < matSecond.materialIdx;
}

// ================================================================================================
static void optionalBufferBinding(
    CmdListHandle cmdList,
    BufferHandle buffer,
    std::string_view name,
    const PipelineInfo* pipInfo,
    uint32_t offset = 0,
    uint32_t range  = 0 )
{
   if( FlatShaderBinding res = pipInfo->findBinding( buffer, name ); res.valid )
   {
      GRIS::BindUniformBuffer( cmdList, buffer, res.binding, res.set, offset, range );
   }
}

static void optionalTextureBinding(
    CmdListHandle cmdList,
    TextureHandle texture,
    std::string_view name,
    const PipelineInfo* pipInfo )
{
   if( FlatShaderBinding res = pipInfo->findBinding( texture, name ); res.valid )
   {
      GRIS::BindTexture( cmdList, texture, res.binding, res.set );
   }
}

// ================================================================================================
void ForwardRenderSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "ForwardRenderSystem" );

   // Start command list recording
   const CmdListHandle cmdList = GRIS::GetMainCommandList();
   CYD_GPUTRACE( cmdList, "ForwardRenderSystem" );

   // Finding main view
   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "MAIN" );
   if( it == scene.viewNames.end() )
   {
      // TODO WARNING
      CYD_ASSERT( !"Could not find main view, skipping render tick" );
      return;
   }
   const uint32_t viewIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

   // TODO TEMP
   if( scene.shadowMap )
   {
      SamplerInfo sampler;
      sampler.useCompare  = true;  // For PCF
      sampler.compare     = CompareOperator::GREATER_EQUAL;
      sampler.addressMode = AddressMode::CLAMP_TO_BORDER;
      sampler.borderColor = BorderColor::OPAQUE_BLACK;
      GRIS::BindTexture( cmdList, scene.shadowMap, sampler, 2, 1 );
   }

   // Rendering straight to swapchain
   GRIS::BeginRendering( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   // Tracking
   std::string_view prevMesh;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );
      if( !renderable.isVisible ||
          renderable.type != RenderableComponent::Type::FORWARD )  // Forward renderables only
      {
         continue;
      }

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
            printf( "ForwardRenderSystem: Passed a null pipeline, skipping entity" );
            continue;
         }

         GRIS::BindPipeline( cmdList, curPipInfo );

         prevPipeline = material.pipelineIdx;
      }

      // Optional Buffers
      // ==========================================================================================
      optionalBufferBinding(
          cmdList, scene.viewsBuffer, "EnvironmentView", curPipInfo, 0, sizeof( scene.views ) );

      optionalBufferBinding( cmdList, scene.lightsBuffer, "Lights", curPipInfo );

      if( renderable.instanceCount )
      {
         CYD_ASSERT( renderable.instancesBuffer && "Invalid Instances Buffer" );
         optionalBufferBinding( cmdList, renderable.instancesBuffer, "InstancesData", curPipInfo );
      }

      // Push Constants
      // ==========================================================================================
      const glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );

      // Update model transform push constant
      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      // TEMPORARY
      /*
      struct PBRConstant
      {
         glm::vec4 color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
         glm::vec4 pbr   = glm::vec4( 0.0f, 0.25f, 1.0f, 0.0f );
      } pbrconstant;

      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::FRAGMENT_STAGE, 64, sizeof( float ) * 8, &pbrconstant );
      */
      // TEMPORARY

      // Material
      // ==========================================================================================
      if( prevMaterial != material.materialIdx )
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
