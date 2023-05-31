#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Physics/MotionComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class PlayerMoveSystem final
    : public CommonSystem<InputComponent, TransformComponent, MotionComponent>
{
  public:
   PlayerMoveSystem() = default;
   NON_COPIABLE( PlayerMoveSystem );
   virtual ~PlayerMoveSystem() = default;

   // Coefficients
   static constexpr float MOVE_ACCELERATION = 100.0f;
   static constexpr float FRICTION_MODIFIER = 0.5f;  // Between 0 and 1
   static constexpr float SPEED_MODIFIER    = 2.0f;  // Sprinting
   static constexpr float MAX_VELOCITY      = 25.0f; // Velocity is clamped to this

   static constexpr float MOUSE_SENS = 0.001f;

   void tick( double deltaS ) override;
};
}
