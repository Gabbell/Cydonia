#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <glm/glm.hpp>

/*
This class is the base class for different types of lights. You must inherit from this class
for the scene system to be able to see the entity. This component is also abstract so you cannot
create entities with it.
*/
namespace CYD
{
class LightComponent final : public BaseComponent
{
  public:
   enum class Type
   {
      DIRECTIONAL,
      POINT,
      SPOT
   };

   LightComponent() = default;
   LightComponent( LightComponent::Type lightType, bool shadows = false )
       : type( lightType ), shadows( shadows )
   {
   }
   COPIABLE( LightComponent );
   virtual ~LightComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::LIGHT;

   Type type = Type::DIRECTIONAL;

   glm::vec4 color = glm::vec4( 1.0f );

   TextureHandle shadowMap;  // Optional

   bool enabled = true;
   bool shadows = false;
};
}