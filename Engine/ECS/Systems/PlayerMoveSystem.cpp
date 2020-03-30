#include <ECS/Systems/PlayerMoveSystem.h>

#include <ECS/ECS.h>

#include <Graphics/Transforms.h>

#include <glm/glm.hpp>

namespace cyd
{
void PlayerMoveSystem::tick( double /*deltaS*/ )
{
   for( const auto& compPair : m_components )
   {
      const InputComponent& input   = ECS::GetSharedComponent<InputComponent>();
      TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );
      MotionComponent& motion       = *std::get<MotionComponent*>( compPair.second );

      // Modifying the transform component directly for rotation
      if( input.rotating )
      {
         glm::vec2 rotationAngles = input.cursorDelta * MOUSE_SENS;

         Transform::rotateLocal( transform.rotation, rotationAngles.y, 0, 0 );
         Transform::rotate( transform.rotation, 0, rotationAngles.x, 0 );
      }

      // Resetting player velocity to not accumulate velocity over time
      motion.velocity = glm::vec3( 0.0f );

      // Modifying the motion component for position in local coordinates
      glm::vec3 delta( 0.0f );
      if( input.goingForwards )
      {
         delta.z -= MOVE_SPEED;
      }
      if( input.goingBackwards )
      {
         delta.z += MOVE_SPEED;
      }
      if( input.goingRight )
      {
         delta.x += MOVE_SPEED;
      }
      if( input.goingLeft )
      {
         delta.x -= MOVE_SPEED;
      }

      Transform::translateLocal( motion.velocity, transform.rotation, delta );

      glm::vec3 elevation( 0.0f );
      if( input.goingUp )
      {
         elevation.y += MOVE_SPEED;
      }
      if( input.goingDown )
      {
         elevation.y -= MOVE_SPEED;
      }

      Transform::translate( motion.velocity, elevation );
   }
}
}
