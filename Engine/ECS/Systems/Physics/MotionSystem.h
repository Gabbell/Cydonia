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
class MotionSystem final : public CommonSystem<TransformComponent, MotionComponent>
{
  public:
   MotionSystem() = default;
   NON_COPIABLE( MotionSystem );
   virtual ~MotionSystem() = default;

   void tick( double deltaS ) override;
};
}