#include <ECS/Systems/Physics/PlayerMoveSystem.h>

#include <ECS/EntityManager.h>

#include <Graphics/Utility/Transforms.h>

#include <glm/glm.hpp>

namespace CYD
{
void PlayerMoveSystem::tick( double deltaS )
{
   for( const auto& entityEntry : m_entities )
   {
      const InputComponent& input   = m_ecs->getSharedComponent<InputComponent>();
      TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      MotionComponent& motion       = GetComponent<MotionComponent>( entityEntry );

      // Modifying the transform component directly for rotation
      if( input.rightClick )
      {
         const glm::vec2 rotationAngles = input.cursorDelta * MOUSE_SENS;
         Transform::RotateLocal( transform.rotation, rotationAngles.y, 0.0, 0.0 );
         Transform::Rotate( transform.rotation, 0.0, rotationAngles.x, 0.0 );
      }

      // Modifying the acceleration component for position in local coordinates
      glm::vec3 accelerationVec( 0.0f );
      if( input.goingForwards )
      {
         accelerationVec.z += 1.0f;
      }
      if( input.goingBackwards )
      {
         accelerationVec.z -= 1.0f;
      }
      if( input.goingRight )
      {
         accelerationVec.x += 1.0f;
      }
      if( input.goingLeft )
      {
         accelerationVec.x -= 1.0f;
      }

      // Rotating unit acceleration vector
      accelerationVec = glm::rotate( transform.rotation, accelerationVec );

      // Elevation is done independently of current orientation
      if( input.goingUp )
      {
         accelerationVec.y += 1.0f;
      }
      if( input.goingDown )
      {
         accelerationVec.y -= 1.0f;
      }

      if( accelerationVec != glm::zero<glm::vec3>() )
      {
         accelerationVec = glm::normalize( accelerationVec );
      }

      accelerationVec *= motion.moveAcceleration;

      // Sprinting
      if( input.sprinting )
      {
         accelerationVec *= SPEED_MODIFIER;
      }

      // Creating friction vector
      glm::vec3 frictionVec( 0.0f );
      if( motion.velocity != glm::zero<glm::vec3>() )
      {
         frictionVec =
             -glm::normalize( motion.velocity ) * motion.moveAcceleration * FRICTION_MODIFIER;
      }

      // Calculating velocity
      motion.velocity += ( accelerationVec * static_cast<float>( deltaS ) ) +
                         ( frictionVec * static_cast<float>( deltaS ) );

      const float magSquared         = glm::length2( motion.velocity );
      const float maxVelocitySquared = motion.maxVelocity * motion.maxVelocity;

      // Rounding to 0 if the velocity is small
      if( magSquared < 0.01 )
      {
         motion.velocity = glm::vec3( 0.0f );
      }

      // Clamping to maximum velocity
      if( magSquared > maxVelocitySquared )
      {
         motion.velocity *= ( maxVelocitySquared / magSquared );
      }

      // Calculating delta position
      const glm::vec3 delta = motion.velocity * static_cast<float>( deltaS );
      Transform::Translate( transform.position, delta );
   }
}
}
