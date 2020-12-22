#include <ECS/Systems/Scene/CameraSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>

#include <glm/gtc/matrix_transform.hpp>

namespace CYD
{
void CameraSystem::tick( double /*deltaS*/ )
{
   if( m_entities.size() > 1 )
   {
      CYDASSERT( !"CameraSystem: Attaching camera to more than one entity not supported" );
      return;
   }

   CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );

      camera.position = glm::vec4( transform.position, 1.0f );

      camera.vp.view = glm::toMat4( glm::conjugate( transform.rotation ) ) *
                       glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / transform.scaling ) *
                       glm::translate( glm::mat4( 1.0f ), -transform.position );

      switch( camera.projMode )
      {
         case CameraComponent::ProjectionMode::PERSPECTIVE:
            camera.vp.proj = glm::perspectiveZO(
                glm::radians( camera.fov ), camera.aspectRatio, camera.near, camera.far );
            break;
         case CameraComponent::ProjectionMode::ORTHOGRAPHIC:
            camera.vp.proj = glm::orthoZO(
                camera.left, camera.right, camera.bottom, camera.top, camera.near, camera.far );
            break;
      }
   }

   // Updating UBOs
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   GRIS::CopyToBuffer(
       camera.viewProjBuffer, &camera.vp, 0, sizeof( CameraComponent::ViewProjection ) );
   GRIS::CopyToBuffer( camera.positionBuffer, &camera.position, 0, sizeof( glm::vec4 ) );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}
}
