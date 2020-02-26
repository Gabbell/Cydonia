#include <ECS/Systems/PlayerMoveSystem.h>

#include <ECS/ECS.h>

#include <glm/glm.hpp>

namespace cyd
{
static void rotate( TransformComponent& transform, float pitch, float yaw, float roll )
{
   glm::quat rotPitch = glm::angleAxis( pitch, glm::vec3( 1, 0, 0 ) );
   glm::quat rotYaw   = glm::angleAxis( yaw, glm::vec3( 0, 1, 0 ) );
   glm::quat rotRoll  = glm::angleAxis( roll, glm::vec3( 0, 0, 1 ) );
   transform.rotation = rotRoll * rotPitch * rotYaw * transform.rotation;
}

static void rotateLocal( TransformComponent& transform, float pitch, float yaw, float roll )
{
   glm::vec3 localXAxis = glm::rotate( transform.rotation, glm::vec3( 1, 0, 0 ) );
   glm::vec3 localYAxis = glm::rotate( transform.rotation, glm::vec3( 0, 1, 0 ) );
   glm::vec3 localZAxis = glm::rotate( transform.rotation, glm::vec3( 0, 0, 1 ) );

   glm::quat rotPitch = glm::angleAxis( pitch, localXAxis );
   glm::quat rotYaw   = glm::angleAxis( yaw, localYAxis );
   glm::quat rotRoll  = glm::angleAxis( roll, localZAxis );

   transform.rotation = rotRoll * rotPitch * rotYaw * transform.rotation;
}

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

         rotateLocal( transform, rotationAngles.y, 0, 0 );
         rotate( transform, 0, rotationAngles.x, 0 );
      }

      // Modifying the motion component for position in local coordinates
      glm::vec3 velocity( 0.0f );
      if( input.goingForwards )
      {
         velocity.z -= MOVE_SPEED;
      }
      if( input.goingBackwards )
      {
         velocity.z += MOVE_SPEED;
      }
      if( input.goingRight )
      {
         velocity.x += MOVE_SPEED;
      }
      if( input.goingLeft )
      {
         velocity.x -= MOVE_SPEED;
      }

      // Converting to world coordinates
      velocity = glm::rotate( transform.rotation, velocity );

      if( input.goingUp )
      {
         velocity.y += MOVE_SPEED;
      }
      if( input.goingDown )
      {
         velocity.y -= MOVE_SPEED;
      }

      motion.velocity = velocity;
   }
}
}
