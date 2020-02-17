#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/MotionComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class PlayerMoveSystem final
    : public CommonSystem<InputComponent, TransformComponent, MotionComponent>
{
  public:
   PlayerMoveSystem() = default;
   NON_COPIABLE( PlayerMoveSystem )
   virtual ~PlayerMoveSystem() = default;

   static constexpr float MOVE_SPEED = 10.0f;
   static constexpr float MOUSE_SENS = 0.001f;

   bool init() override { return true; };
   void uninit() override{};
   void tick( double deltaS ) override;
};
}
