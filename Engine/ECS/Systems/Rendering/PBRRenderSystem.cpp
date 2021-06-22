#include <ECS/Systems/Rendering/PBRRenderSystem.h>

#include <Graphics/RenderGraph.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t ENV_VIEW_SET = 0;

static void setupScene( CmdListHandle cmdList )
{
   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
   const SceneComponent& scene   = ECS::GetSharedComponent<SceneComponent>();

   GRIS::BindUniformBuffer( cmdList, camera.viewBuffer, ENV_VIEW_SET, 0 );
   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, ENV_VIEW_SET, 2 );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );
}

bool PBRRenderSystem::_compareEntities( const EntityEntry& first, const EntityEntry& second )
{
   // We are sorting entities by their pipeline
   // TODO Sort entities better by shader/material
   const MaterialComponent& matFirst  = *std::get<MaterialComponent*>( first.arch );
   const MaterialComponent& matSecond = *std::get<MaterialComponent*>( second.arch );
   return matFirst.data.pipeline < matSecond.data.pipeline;
}

void PBRRenderSystem::tick( double /*deltaS*/ )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, "PBRRenderSystem", true );

   GRIS::StartRecordingCommandList( cmdList );

   setupScene( cmdList );

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
      if( prevPipeline != material.data.pipeline )
      {
         pipInfo = RenderPipelines::Get( material.data.pipeline );
         if( pipInfo == nullptr )
         {
            // TODO WARNINGS
            printf( "PBRRenderSystem: Passed a null pipeline, skipping entity" );
            continue;
         }

         GRIS::BindPipeline( cmdList, pipInfo );
         prevPipeline = material.data.pipeline;
      }

      // Update model transform push constant
      // ==========================================================================================
      GRIS::UpdateConstantBuffer(
          cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      // Bind material
      // ==========================================================================================
      if( material.data.albedo )
      {
         GRIS::BindTexture( cmdList, material.data.albedo, "albedo" );
      }
      if( material.data.normals )
      {
         GRIS::BindTexture( cmdList, material.data.normals, "normals" );
      }
      if( material.data.metalness )
      {
         GRIS::BindTexture( cmdList, material.data.metalness, "metalness" );
      }
      if( material.data.roughness )
      {
         GRIS::BindTexture( cmdList, material.data.roughness, "roughness" );
      }
      if( material.data.ao )
      {
         GRIS::BindTexture( cmdList, material.data.ao, "ambientOcclusion" );
      }
      if( material.data.disp )
      {
         GRIS::BindTexture( cmdList, material.data.disp, "displacement" );
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
