#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/SharedComponents/SharedComponentType.h>

#include <glm/glm.hpp>

namespace CYD
{
class SceneComponent final : public BaseSharedComponent
{
  public:
   SceneComponent();
   NON_COPIABLE( SceneComponent );
   virtual ~SceneComponent();

   static constexpr SharedComponentType TYPE = SharedComponentType::SCENE;

   // TODO Resize
   Viewport viewport = { 0.0f, 1080.0f, 1920.0f, -1080.0f };
   Rectangle scissor = { 0, 0, 1920, 1080 };

   struct DirectionalLightUBO
   {
      glm::vec4 enabled;
      glm::vec4 direction;
      glm::vec4 color;
   } dirLight = {};

   BufferHandle lightsBuffer;

   TextureHandle shadowMap;
};
}
