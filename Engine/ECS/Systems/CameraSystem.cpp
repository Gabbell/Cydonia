#include <ECS/Systems/CameraSystem.h>

#include <ECS/ECS.h>
#include <ECS/SharedComponents/CameraComponent.h>

#include <glm/gtc/matrix_transform.hpp>

namespace cyd
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

      switch( _projMode )
      {
         case ProjectionMode::PERSPECTIVE:
            camera.vp.proj = glm::perspectiveZO( glm::radians( _fov ), _aspectRatio, _near, _far );
            break;
         case ProjectionMode::ORTHOGRAPHIC:
            camera.vp.proj = glm::orthoZO( _left, _right, _bottom, _top, _near, _far );
            break;
         default:
            CYDASSERT( !"Unrecognized camera projection mode" );
      }
   }
}
}
