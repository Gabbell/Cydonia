#pragma once

#include <ECS/Components/Lights/LightComponent.h>

#include <ECS/Components/Lights/LightTypes.h>

namespace CYD
{
class DirectionalLightComponent final : public LightComponent
{
  public:
   DirectionalLightComponent() : LightComponent( LightType::DIRECTIONAL ) {}
   COPIABLE( DirectionalLightComponent )
   virtual ~DirectionalLightComponent() = default;
};
}