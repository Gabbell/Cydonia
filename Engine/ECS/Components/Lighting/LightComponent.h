#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

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
   LightComponent() = default;
   COPIABLE( LightComponent );
   virtual ~LightComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::LIGHT;

   glm::vec4 position  = glm::vec4( 0.0f );
   glm::vec4 direction = glm::vec4( 0.0f );
   glm::vec4 color     = glm::vec4( 1.0f );
   bool enabled        = false;
};
}