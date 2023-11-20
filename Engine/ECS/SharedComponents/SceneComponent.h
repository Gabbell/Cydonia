#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Scene/Frustum.h>
#include <Graphics/Utility/GBuffer.h>

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

   // Main Render Targets
   // =============================================================================================
   Framebuffer mainFramebuffer;
   TextureHandle mainColor;
   TextureHandle mainDepth;

   // State
   // =============================================================================================
   Extent2D extent;
   Viewport viewport = { 0.0f, 0.0f, 0.0f, 0.0f };
   Rectangle scissor = { { 0, 0 }, { 0, 0 } };

   // Views
   // =============================================================================================
   static constexpr uint32_t MAX_VIEWS = 8;

   struct ViewShaderParams
   {
      glm::vec4 position;
      glm::mat4 viewMat;
      glm::mat4 projMat;
   };

   struct InverseViewShaderParams
   {
      glm::mat4 invViewMat;
      glm::mat4 invProjMat;
   };

   std::array<std::string, MAX_VIEWS> viewNames;
   ViewShaderParams views[MAX_VIEWS]               = {};
   InverseViewShaderParams inverseViews[MAX_VIEWS] = {};

   Frustum frustums[MAX_VIEWS] = {};

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

   // Ressource Handles
   // =============================================================================================
   BufferHandle viewsBuffer;
   BufferHandle inverseViewsBuffer;
   BufferHandle lightsBuffer;

   TextureHandle shadowMap;  // TODO This shouldn't be here, not a very elegant solution
   TextureHandle envMap;

   GBuffer gbuffer;

#if CYD_DEBUG
   BufferHandle debugParamsBuffer;
#endif

   // State Tackers
   // =============================================================================================
   bool resolutionChanged = true;
};
}
