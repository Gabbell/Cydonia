#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>

namespace CYD
{
class MotionComponent final : public BaseComponent
{
  public:
   MotionComponent() = default;
   MotionComponent( float moveAcceleration, float maxVelocity )
       : moveAcceleration( moveAcceleration ), maxVelocity( maxVelocity )
   {
   }
   COPIABLE( MotionComponent );
   virtual ~MotionComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MOTION;

   // These are pretty good defaults for a 1 unit == 1 meter scenario
   static constexpr float DEFAULT_MOVE_ACCELERATION = 100.0f;
   static constexpr float DEFAULT_MAX_VELOCITY      = 10.0f;

   // Constants
   float moveAcceleration = DEFAULT_MOVE_ACCELERATION;
   float maxVelocity      = DEFAULT_MAX_VELOCITY;

   // Current
   glm::vec3 velocity     = glm::vec3( 0.0f );
   glm::vec3 acceleration = glm::vec3( 0.0f );
};
}
