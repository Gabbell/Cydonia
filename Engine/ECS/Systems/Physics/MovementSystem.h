#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Physics/MotionComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MovementSystem final : public CommonSystem<TransformComponent, MotionComponent>
{
  public:
   MovementSystem() = default;
   NON_COPIABLE( MovementSystem )
   virtual ~MovementSystem() = default;

   bool init() override { return true; };
   void uninit() override{};
   void tick( double deltaS ) override;
};
}