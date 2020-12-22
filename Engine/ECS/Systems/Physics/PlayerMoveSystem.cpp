#include <ECS/Systems/Physics/PlayerMoveSystem.h>

#include <ECS/EntityManager.h>

#include <Graphics/Utility/Transforms.h>

#include <glm/glm.hpp>

namespace CYD
{
void PlayerMoveSystem::tick( double /*deltaS*/ )
{
   for( const auto& entityEntry : m_entities )
   {
      const InputComponent& input   = ECS::GetSharedComponent<InputComponent>();
      TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      MotionComponent& motion       = *std::get<MotionComponent*>( entityEntry.arch );

      // Modifying the transform component directly for rotation
      if( input.rotating )
      {
         glm::vec2 rotationAngles = input.cursorDelta * MOUSE_SENS;

         Transform::RotateLocal( transform.rotation, rotationAngles.y, 0, 0 );
         Transform::Rotate( transform.rotation, 0, rotationAngles.x, 0 );
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

      Transform::TranslateLocal( motion.velocity, transform.rotation, delta );

      glm::vec3 elevation( 0.0f );
      if( input.goingUp )
      {
         elevation.y += MOVE_SPEED;
      }
      if( input.goingDown )
      {
         elevation.y -= MOVE_SPEED;
      }

      Transform::Translate( motion.velocity, elevation );
   }
}
}
