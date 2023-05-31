#include <ECS/Systems/Lighting/ShadowMapSystem.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
/*
static void setupScene( EntityManager* ecs, CmdListHandle cmdList )
{
   SceneComponent& scene = ecs->getSharedComponent<SceneComponent>();

   if( !scene.shadowMap )
   {
      TextureDescription texDesc;
      texDesc.width  = static_cast<uint32_t>( std::abs( scene.viewport.width ) );
      texDesc.height = static_cast<uint32_t>( std::abs( scene.viewport.height ) );
      texDesc.size   = texDesc.width * texDesc.height * sizeof( float );
      texDesc.type   = ImageType::TEXTURE_2D;
      texDesc.format = PixelFormat::D32_SFLOAT;
      texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::DEPTH_STENCIL;
      texDesc.stages = PipelineStage::FRAGMENT_STAGE;
      texDesc.name   = "Shadow Map";

      scene.shadowMap = GRIS::CreateTexture( texDesc );
   }

   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 0, 1 );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );
}
*/

void ShadowMapSystem::tick( double /*deltaS*/ )
{
   /*
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS | TRANSFER );

   GRIS::StartRecordingCommandList( cmdList );

   setupScene( m_ecs, cmdList );

   FramebufferInfo framebuffer;
   framebuffer.attachments.push_back(
       { PixelFormat::D32_SFLOAT, AttachmentType::DEPTH_STENCIL, LoadOp::CLEAR, StoreOp::STORE } );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   GRIS::BeginRendering( cmdList, framebuffer, { scene.shadowMap } );

   GRIS::BindPipeline( cmdList, "SHADOWMAP_GENERATION" );

   std::string_view prevMesh;
   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );

      if( renderable.isOccluder )
      {
         const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
         const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

         const glm::mat4 modelMatrix = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                                       glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                                       glm::toMat4( transform.rotation );

         // Update model transform push constant
         GRIS::UpdateConstantBuffer(
             cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

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
            GRIS::DrawIndexed( cmdList, mesh.indexCount );
         }
         else
         {
            GRIS::Draw( cmdList, mesh.vertexCount );
         }
      }
   }

   GRIS::EndRendering( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::WaitOnCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );
   */
}
}