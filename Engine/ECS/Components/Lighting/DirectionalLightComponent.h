#pragma once

#include <ECS/Components/Lighting/LightComponent.h>

#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
class DirectionalLightComponent final : public LightComponent
{
  public:
   DirectionalLightComponent() = default;
   COPIABLE( DirectionalLightComponent )
   virtual ~DirectionalLightComponent() = default;

   ComponentType getType() const override { return SUBTYPE; }

   static constexpr ComponentType SUBTYPE = ComponentType::LIGHT_DIRECTIONAL;
};
}