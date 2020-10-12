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
class LightSystem final : public CommonSystem<TransformComponent, LightComponent>
{
  public:
   LightSystem() = default;
   NON_COPIABLE( LightSystem );
   virtual ~LightSystem() = default;

   void tick( double deltaS ) override;
};
}