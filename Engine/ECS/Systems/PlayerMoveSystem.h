#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/Components/MotionComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class PlayerMoveSystem final : public CommonSystem<InputComponent, MotionComponent>
{
  public:
   PlayerMoveSystem() = default;
   NON_COPIABLE( PlayerMoveSystem )
   virtual ~PlayerMoveSystem() = default;

   bool init() override { return true; };
   void uninit() override{};
   void tick( double deltaS ) override;
};
}
