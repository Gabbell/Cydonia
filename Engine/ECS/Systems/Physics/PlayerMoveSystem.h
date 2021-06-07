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

   static constexpr float MOVE_SPEED = 5.0f;
   static constexpr float MOUSE_SENS = 0.001f;

   void tick( double deltaS ) override;
};
}
