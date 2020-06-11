#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Graphics/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/RenderPipelines.h>

#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static Rectangle viewport;
static constexpr uint32_t ENV_VIEW = 0;  // Environment/View Set

void RenderSystem::tick( double deltaS )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
   const SceneComponent& scene   = ECS::GetSharedComponent<SceneComponent>();

   // Camera
   GRIS::CopyToBuffer( viewProjectionBuffer, &camera.vp, 0, sizeof( camera.vp ) );
   GRIS::CopyToBuffer( cameraPosBuffer, &camera.pos, 0, sizeof( camera.pos ) );
   GRIS::BindUniformBuffer( cmdList, viewProjectionBuffer, ENV_VIEW, 0 );
   GRIS::BindUniformBuffer( cmdList, cameraPosBuffer, ENV_VIEW, 1 );

   // Scene
   GRIS::CopyToBuffer( lightsBuffer, &scene.dirLight, 0, sizeof( scene.dirLight ) );
   GRIS::BindUniformBuffer( cmdList, lightsBuffer, ENV_VIEW, 2 );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, viewport );

   // Main pass
   GRIS::BeginRenderPassSwapchain( cmdList, true );

   // TODO Either somehow sort renderables by PSO or find a way to decrease calls to BindPipeline
   // as much as possible to reduce overhead.
   for( const auto& compPair : m_components )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );

      const glm::mat4 modelMatrix = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                                    glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                                    glm::toMat4( transform.rotation );

      const RenderableComponent* renderable = std::get<RenderableComponent*>( compPair.second );
      RenderPipelines::Prepare( cmdList, deltaS, modelMatrix, renderable );

      const MeshComponent& mesh = *std::get<MeshComponent*>( compPair.second );
      GRIS::BindVertexBuffer( cmdList, mesh.vertexBuffer );

      const bool hasIndexBuffer = ( mesh.indexBuffer != Handle::INVALID_HANDLE );
      if( hasIndexBuffer )
      {
         // This renderable has an index buffer, use it to draw
         GRIS::BindIndexBuffer<uint32_t>( cmdList, mesh.indexBuffer );
         GRIS::DrawVerticesIndexed( cmdList, mesh.indexCount );
      }
      else
      {
         GRIS::DrawVertices( cmdList, mesh.vertexCount );
      }
   }

   GRIS::EndRenderPass( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();
}

bool RenderSystem::init()
{
   // Flipping Y in viewport since we want Y to be up (like GL)
   viewport.offsetX = 0.0f;
   viewport.offsetY = 1080;
   viewport.width   = 1920;
   viewport.height  = -1080;

   viewProjectionBuffer = GRIS::CreateUniformBuffer( sizeof( glm::mat4 ) * 2 );
   cameraPosBuffer      = GRIS::CreateUniformBuffer( sizeof( glm::vec4 ) );
   lightsBuffer = GRIS::CreateUniformBuffer( sizeof( SceneComponent::DirectionalLightUBO ) );

   return true;
}

void RenderSystem::uninit()
{
   GRIS::DestroyBuffer( lightsBuffer );
   GRIS::DestroyBuffer( cameraPosBuffer );
   GRIS::DestroyBuffer( viewProjectionBuffer );
}
}
