#pragma once

#include <ECS/Components/Lighting/LightComponent.h>

#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
class PointLightComponent final : public LightComponent
{
  public:
   PointLightComponent() = default;
   COPIABLE( PointLightComponent )
   virtual ~PointLightComponent() = default;

   ComponentType getType() const override { return SUBTYPE; }

   static constexpr ComponentType SUBTYPE = ComponentType::LIGHT_POINT;
};
}