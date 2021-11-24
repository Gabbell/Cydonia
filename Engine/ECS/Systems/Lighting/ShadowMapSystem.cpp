#include <ECS/Systems/Lighting/ShadowMapSystem.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t ENV_VIEW_SET = 0;

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
      texDesc.stages = ShaderStage::FRAGMENT_STAGE;
      texDesc.name   = "Shadow Map";

      scene.shadowMap = GRIS::CreateTexture( texDesc );
   }

   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, ENV_VIEW_SET, 0 );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );
}

void ShadowMapSystem::tick( double /*deltaS*/ )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS | TRANSFER );

   GRIS::StartRecordingCommandList( cmdList );

   setupScene( m_ecs, cmdList );

   RenderTargetsInfo targetsInfo;
   targetsInfo.attachments.push_back(
       { PixelFormat::D32_SFLOAT, AttachmentType::DEPTH_STENCIL, LoadOp::CLEAR, StoreOp::STORE } );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   GRIS::BeginRendering( cmdList, targetsInfo, { scene.shadowMap } );

   GRIS::BindPipeline( cmdList, "SHADOWMAP_GENERATION" );

   std::string_view prevMesh;
   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      const glm::mat4 modelMatrix = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                                    glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                                    glm::toMat4( transform.rotation );

      // Update model transform push constant
      GRIS::UpdateConstantBuffer(
          cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

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

   GRIS::SubmitCommandList( cmdList );
   GRIS::WaitOnCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );
}
}