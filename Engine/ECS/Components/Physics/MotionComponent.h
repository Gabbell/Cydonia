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

   // Constants
   float moveAcceleration = 0.0f;
   float maxVelocity      = 0.0f;

   // Current
   glm::vec3 velocity     = glm::vec3( 0.0f );
   glm::vec3 acceleration = glm::vec3( 0.0f );
};
}
