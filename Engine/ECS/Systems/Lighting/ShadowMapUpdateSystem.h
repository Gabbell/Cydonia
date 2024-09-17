#pragma once
#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Lighting/ShadowMapComponent.h>
#include <ECS/Components/Scene/ViewComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class ShadowMapUpdateSystem final
    : public CommonSystem<TransformComponent, ViewComponent, ShadowMapComponent>
{
  public:
   ShadowMapUpdateSystem() = default;
   NON_COPIABLE( ShadowMapUpdateSystem );
   virtual ~ShadowMapUpdateSystem() = default;

   void tick( double deltaS ) override;
};
}