#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <glm/glm.hpp>

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

   bool enabled = true;
   bool shadows = false;
};
}