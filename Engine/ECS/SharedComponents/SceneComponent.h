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

   struct LightUBO
   {
      glm::mat4 viewMat;
      glm::vec4 position;
      glm::vec4 color;
      glm::vec4 enabled;
   } light = {};

   BufferHandle lightsBuffer;

   TextureHandle shadowMap;
};
}
