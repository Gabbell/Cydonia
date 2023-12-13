#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Utility/ShaderStructs.h>
#include <Graphics/Utility/GBuffer.h>

#include <ECS/Components/Lighting/LightComponent.h>

#include <ECS/SharedComponents/SharedComponentType.h>

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

   // TODO
   // Some of these buffers could be host-side buffers (staging) and we could be writing into them
   // right away

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

   std::array<std::string, MAX_VIEWS> viewNames    = {};
   ViewShaderParams views[MAX_VIEWS]               = {};
   InverseViewShaderParams inverseViews[MAX_VIEWS] = {};
   FrustumShaderParams frustums[MAX_VIEWS]         = {};

   // Lights
   // =============================================================================================
   static constexpr uint32_t MAX_LIGHTS = 3;

   std::array<std::string, MAX_LIGHTS> lightNames = {};
   LightShaderParams lights[MAX_LIGHTS]           = {};

   // Shadows
   // =============================================================================================
   static constexpr uint32_t MAX_SHADOW_MAPS = 3;

   std::array<std::string, MAX_SHADOW_MAPS> shadowMapNames = {};
   ShadowMapShaderParams shadowMaps[MAX_SHADOW_MAPS]       = {};
   TextureHandle shadowMapTextures[MAX_SHADOW_MAPS]        = {};
   Framebuffer shadowMapFBs[MAX_SHADOW_MAPS]               = {};

   // Ressource Handles
   // =============================================================================================
   BufferHandle viewsBuffer;
   BufferHandle inverseViewsBuffer;
   BufferHandle frustumsBuffer;
   BufferHandle lightsBuffer;
   BufferHandle shadowMapsBuffer;

   TextureHandle quarterResShadowMask;  // Used for raymarching
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
