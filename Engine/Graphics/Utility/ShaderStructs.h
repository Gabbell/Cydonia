#pragma once

#include <Graphics/Scene/Frustum.h>
#include <Graphics/Utility/ShaderMemoryLayout.h>
#include <Graphics/Utility/ShadowMapping.h>

#include <glm/glm.hpp>

namespace CYD
{
// Views
// =============================================================================================
struct ViewShaderParams
{
   glm::vec4 position;
   glm::mat4 viewMat;
   glm::mat4 projMat;
};

struct InverseViewShaderParams
{
   glm::vec4 position;
   glm::mat4 invViewMat;
   glm::mat4 invProjMat;
};

struct FrustumShaderParams
{
   glm::vec4 planes[Frustum::PLANE_COUNT];
};

// Lights
// =============================================================================================
struct LightShaderParams
{
   glm::vec4 position;
   glm::vec4 direction;
   glm::vec4 color;
   glm::vec4 enabled;
};

// Shadows
// =============================================================================================
struct ShadowCascade
{
   float farDepth;
   float radius;
   float texelSize;
   CYD_SHADER_ALIGN glm::mat4 worldToLightMatrix;
   glm::vec4 planes[Frustum::PLANE_COUNT];
};

struct ShadowMapShaderParams
{
   ShadowCascade cascade[ShadowMapping::MAX_CASCADES];
   bool enabled = false;
};
}