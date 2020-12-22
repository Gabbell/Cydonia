#include <ECS/Systems/Rendering/PBRRenderSystem.h>

#include <Graphics/RenderPipelines.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t ENV_VIEW_SET = 0;
static constexpr uint32_t MATERIAL_SET = 1;

static void setupScene( CmdListHandle cmdList )
{
   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
   const SceneComponent& scene   = ECS::GetSharedComponent<SceneComponent>();

   GRIS::BindUniformBuffer( cmdList, camera.viewProjBuffer, ENV_VIEW_SET, 0 );
   GRIS::BindUniformBuffer( cmdList, camera.positionBuffer, ENV_VIEW_SET, 1 );
   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, ENV_VIEW_SET, 2 );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );
}

void PBRRenderSystem::tick( double /*deltaS*/ )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   GRIS::StartRecordingCommandList( cmdList );

   setupScene( cmdList );

   // Rendering straight to swapchain with depth attachment
   GRIS::BeginRendering( cmdList, true );

   // TODO Sort PSOs by shader/material
   std::string_view prevMesh;
   std::string_view prevPipeline;
   std::string_view prevMaterial;

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MaterialComponent& material   = *std::get<MaterialComponent*>( entityEntry.arch );

      if( prevPipeline != material.pipeline )
      {
         GRIS::BindPipeline( cmdList, material.pipeline );
         prevPipeline = material.pipeline;
      }

      // Update model transform push constant
      const glm::mat4 modelMatrix = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                                    glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                                    glm::toMat4( transform.rotation );

      GRIS::UpdateConstantBuffer(
          cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      if( prevMaterial != material.asset )
      {
         if( material.albedo )
         {
            GRIS::BindTexture( cmdList, material.albedo, "albedo" );
         }
         if( material.normals )
         {
            GRIS::BindTexture( cmdList, material.normals, "normals" );
         }
         if( material.metalness )
         {
            GRIS::BindTexture( cmdList, material.metalness, "metalness" );
         }
         if( material.roughness )
         {
            GRIS::BindTexture( cmdList, material.roughness, "roughness" );
         }
         if( material.ao )
         {
            GRIS::BindTexture( cmdList, material.ao, "ambientOcclusion" );
         }
         if( material.height )
         {
            GRIS::BindTexture( cmdList, material.height, "height" );
         }

         prevMaterial = material.asset;
      }

      const MeshComponent& mesh = *std::get<MeshComponent*>( entityEntry.arch );
      if( prevMesh != mesh.asset )
      {
         if( mesh.vertexBuffer )
         {
            GRIS::BindVertexBuffer( cmdList, mesh.vertexBuffer );
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

   GRIS::SubmitCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();
}
}
