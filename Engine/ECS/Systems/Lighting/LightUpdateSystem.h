#pragma once
#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Lighting/LightComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class LightUpdateSystem final : public CommonSystem<TransformComponent, LightComponent>
{
  public:
   LightUpdateSystem() = default;
   NON_COPIABLE( LightUpdateSystem );
   virtual ~LightUpdateSystem() = default;

   void tick( double deltaS ) override;
};
}