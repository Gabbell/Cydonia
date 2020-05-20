#pragma once

#include <ECS/Components/Lights/LightComponent.h>

#include <ECS/Components/Lights/LightTypes.h>

namespace CYD
{
class PointLightComponent final : public LightComponent
{
  public:
   PointLightComponent() : LightComponent( LightType::POINT ) {}
   COPIABLE( PointLightComponent )
   virtual ~PointLightComponent() = default;
};
}