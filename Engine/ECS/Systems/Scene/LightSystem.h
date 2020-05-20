#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Lights/LightComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class LightSystem final : public CommonSystem<TransformComponent, LightComponent>
{
  public:
   LightSystem() = default;
   NON_COPIABLE( LightSystem )
   virtual ~LightSystem() = default;

   bool init() override { return true; }
   void uninit() override{};

   void tick( double deltaS ) override;
};
}