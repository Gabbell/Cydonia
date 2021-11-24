#include <ECS/Systems/Rendering/PBRRenderSystem.h>

#include <Graphics/RenderGraph.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t ENV_VIEW_SET = 0;

bool PBRRenderSystem::_compareEntities( const EntityEntry& first, const EntityEntry& second )
{
   // We are sorting entities by their pipeline
   // TODO Sort entities better by shader/material
   const MaterialComponent& matFirst  = *std::get<MaterialComponent*>( first.arch );
   const MaterialComponent& matSecond = *std::get<MaterialComponent*>( second.arch );
   return matFirst.data.pipeline < matSecond.data.pipeline;
}

static void optionalTextureBinding(
    CmdListHandle cmdList,
    TextureHandle texture,
    std::string_view name,
    const PipelineInfo* pipInfo )
{
   if( ResourceBinding<TextureHandle> res = pipInfo->findBinding( texture, name ); res.valid )
   {
      GRIS::BindTexture( cmdList, res.resource, res.set, res.binding );
   }
}

static void optionalBufferBinding(
    CmdListHandle cmdList,
    BufferHandle buffer,
    std::string_view name,
    const PipelineInfo* pipInfo )
{
   if( ResourceBinding<BufferHandle> res = pipInfo->findBinding( buffer, name ); res.valid )
   {
      GRIS::BindUniformBuffer( cmdList, res.resource, res.set, res.binding );
   }
}

void PBRRenderSystem::tick( double /*deltaS*/ )
{
   const CameraComponent& camera = m_ecs->getSharedComponent<CameraComponent>();
   const SceneComponent& scene   = m_ecs->getSharedComponent<SceneComponent>();

   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, "PBRRenderSystem", true );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );

   // Rendering straight to swapchain
   GRIS::BeginRendering( cmdList );

   std::string_view prevMesh;
   std::string_view prevPipeline;
   std::string_view prevMaterial;

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MaterialComponent& material   = *std::get<MaterialComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      const glm::mat4 modelMatrix = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                                    glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                                    glm::toMat4( transform.rotation );

      // Pipeline
      // ==========================================================================================
      const PipelineInfo* pipInfo = nullptr;

      pipInfo = RenderPipelines::Get( material.data.pipeline );
      if( pipInfo == nullptr )
      {
         // TODO WARNINGS
         printf( "PBRRenderSystem: Passed a null pipeline, skipping entity" );
         continue;
      }

      if( prevPipeline != material.data.pipeline )
      {
         GRIS::BindPipeline( cmdList, pipInfo );
         prevPipeline = material.data.pipeline;
      }

      // Update model transform push constant
      // ==========================================================================================
      GRIS::UpdateConstantBuffer(
          cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      // Optional scene bindings
      // ==========================================================================================
      optionalBufferBinding( cmdList, camera.viewBuffer, "EnvironmentView", pipInfo );
      optionalBufferBinding( cmdList, scene.lightsBuffer, "DirectionalLight", pipInfo );

      // Optional material bindings
      // ==========================================================================================
      optionalTextureBinding( cmdList, material.data.albedo, "albedo", pipInfo );
      optionalTextureBinding( cmdList, material.data.normals, "normals", pipInfo );
      optionalTextureBinding( cmdList, material.data.metalness, "metalness", pipInfo );
      optionalTextureBinding( cmdList, material.data.roughness, "roughness", pipInfo );
      optionalTextureBinding( cmdList, material.data.ao, "ambientOcclusion", pipInfo );
      optionalTextureBinding( cmdList, material.data.disp, "displacement", pipInfo );

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

      if( mesh.indexCount > 0 )
      {
         GRIS::DrawVerticesIndexed( cmdList, mesh.indexCount );
      }
      else
      {
         GRIS::DrawVertices( cmdList, mesh.vertexCount );
      }
   }

   GRIS::EndRendering( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   RenderGraph::AddPass( cmdList );
}
}
