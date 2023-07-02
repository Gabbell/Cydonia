#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Transforms/InstancedComponent.h>

namespace CYD
{
class InstanceUpdateSystem final : public CommonSystem<RenderableComponent, InstancedComponent>
{
  public:
   InstanceUpdateSystem() = default;
   NON_COPIABLE( InstanceUpdateSystem );
   virtual ~InstanceUpdateSystem() = default;

   void tick( double deltaS ) override;
};
}
