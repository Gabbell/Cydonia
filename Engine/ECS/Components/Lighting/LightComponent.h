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
   LightComponent(
       LightComponent::Type lightType,
       const glm::vec4& color,
       const glm::vec3& direction )
       : type( lightType ), color( color ), direction( glm::normalize( direction ) )
   {
   }
   COPIABLE( LightComponent );
   virtual ~LightComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::LIGHT;

   using LightIndex                                = uint32_t;
   static constexpr LightIndex INVALID_LIGHT_INDEX = 0xFFFFFFFF;
   uint32_t index                                  = INVALID_LIGHT_INDEX;

   Type type = Type::DIRECTIONAL;

   glm::vec4 color     = glm::vec4( 1.0f );
   glm::vec3 direction = glm::vec3( 0.0, 0.0, 1.0 );

   bool enabled = true;
};
}