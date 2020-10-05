#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <glm/glm.hpp>

namespace CYD
{
class SceneComponent final : public BaseSharedComponent
{
  public:
   SceneComponent() = default;
   NON_COPIABLE( SceneComponent );
   virtual ~SceneComponent() = default;

   static constexpr SharedComponentType TYPE = SharedComponentType::SCENE;

   struct DirectionalLightUBO
   {
      glm::vec4 enabled;
      glm::vec4 direction;
      glm::vec4 color;
   } dirLight = {};
};
}
