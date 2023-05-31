#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/SharedComponents/SharedComponentType.h>

#include <glm/glm.hpp>

#include <array>
#include <string_view>

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
   Viewport viewport = { 0.0f, 0.0f, 0.0f, 0.0f };
   Rectangle scissor = { 0, 0, 0, 0 };

   // UBO Data
   struct LightUBO
   {
      glm::mat4 viewMat;
      glm::vec4 position;
      glm::vec4 color;
      glm::vec4 enabled;
   };

   struct ViewUBO
   {
      glm::vec4 position;
      glm::mat4 viewMat;
      glm::mat4 projMat;
   };

   static constexpr uint32_t MAX_VIEWS  = 8;
   static constexpr uint32_t MAX_LIGHTS = 1;

   ViewUBO views[MAX_VIEWS]    = {};
   LightUBO lights[MAX_LIGHTS] = {};

   // Ressource Handles
   BufferHandle lightsBuffer;
   BufferHandle viewsBuffer;

#if CYD_DEBUG
   BufferHandle debugParamsBuffer;
#endif

   // Scene Tracking
   std::array<std::string_view, MAX_VIEWS> viewNames;
};
}
