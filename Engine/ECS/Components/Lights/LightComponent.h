#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>
#include <ECS/Components/Lights/LightTypes.h>

#include <glm/glm.hpp>

/*
This class is the base class for different types of lights. You must inherit from this class
for the scene system to be able to see the entity. This component is also abstract so you cannot
create entities with it.
*/
namespace CYD
{
class LightComponent : public BaseComponent
{
  public:
   COPIABLE( LightComponent )
   virtual ~LightComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::LIGHT;

   LightType type = LightType::UNKNOWN;

   bool init() { return true; }
   void uninit() override {}

   glm::vec4 color = glm::vec4( 1.0f );
   bool enabled    = false;

  protected:
   LightComponent() = default;
   LightComponent( LightType aType ) : type( aType ) {}
};
}