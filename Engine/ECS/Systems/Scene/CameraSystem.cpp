#include <ECS/Systems/Scene/CameraSystem.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>

#include <glm/gtc/matrix_transform.hpp>

namespace CYD
{
void CameraSystem::tick( double /*deltaS*/ )
{
   if( m_components.size() > 1 )
   {
      CYDASSERT( !"Attempting to attach camera to more than one entity" );
      return;
   }

   CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();

   for( const auto& compPair : m_components )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );

      camera.pos = glm::vec4( transform.position, 1.0f );

      camera.vp.view = glm::toMat4( glm::conjugate( transform.rotation ) ) *
                       glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / transform.scaling ) *
                       glm::translate( glm::mat4( 1.0f ), -transform.position );

      switch( m_projMode )
      {
         case ProjectionMode::PERSPECTIVE:
            camera.vp.proj =
                glm::perspectiveZO( glm::radians( m_fov ), m_aspectRatio, m_near, m_far );
            break;
         case ProjectionMode::ORTHOGRAPHIC:
            camera.vp.proj = glm::orthoZO( m_left, m_right, m_bottom, m_top, m_near, m_far );
            break;
         default:
            CYDASSERT( !"Unrecognized camera projection mode" );
      }
   }
}
}
