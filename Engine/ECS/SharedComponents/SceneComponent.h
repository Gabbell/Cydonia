#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Scene/Frustum.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/SharedComponents/SharedComponentType.h>

#include <glm/glm.hpp>

#include <array>

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

   // Views
   // =============================================================================================
   static constexpr uint32_t MAX_VIEWS = 8;

   struct ViewShaderParams
   {
      glm::vec4 position;
      glm::mat4 viewMat;
      glm::mat4 projMat;
   };

   std::array<std::string, MAX_VIEWS> viewNames;
   Frustum frustums[MAX_VIEWS]       = {};
   ViewShaderParams views[MAX_VIEWS] = {};

   // Lights
   // =============================================================================================
   static constexpr uint32_t MAX_LIGHTS = 3;

   struct LightShaderParams
   {
      glm::vec4 position;
      glm::vec4 direction;
      glm::vec4 color;
      glm::vec4 enabled;
   };

   LightShaderParams lights[MAX_LIGHTS] = {};

   // =============================================================================================

   // Ressource Handles
   BufferHandle viewsBuffer;
   BufferHandle lightsBuffer;

   // TODO This shouldn't be here, not a very elegant solution
   TextureHandle shadowMap;

#if CYD_DEBUG
   BufferHandle debugParamsBuffer;
#endif
};
}
