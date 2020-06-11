#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Behaviour/EntityFollowComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class EntityFollowSystem final
    : public CommonSystem<TransformComponent, EntityFollowComponent>
{
  public:
   EntityFollowSystem() = default;
   NON_COPIABLE( EntityFollowSystem )
   virtual ~EntityFollowSystem() = default;

   bool init() override { return true; };
   void uninit() override{};
   void tick( double deltaS ) override;
};
}
