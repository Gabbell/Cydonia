#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Scene/ViewComponent.h>

namespace CYD
{
class ViewUpdateSystem final : public CommonSystem<TransformComponent, ViewComponent>
{
  public:
   ViewUpdateSystem() = default;
   NON_COPIABLE( ViewUpdateSystem );
   virtual ~ViewUpdateSystem() = default;

   void tick( double deltaS ) override;

  protected:
   bool _compareEntities( const EntityEntry& first, const EntityEntry& second ) override;
};
}
