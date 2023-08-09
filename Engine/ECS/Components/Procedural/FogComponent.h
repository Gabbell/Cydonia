#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class FogComponent final : public BaseComponent
{
  public:
   FogComponent() = default;
   COPIABLE( FogComponent );
   virtual ~FogComponent();

   static constexpr ComponentType TYPE = ComponentType::FOG;

   struct ViewInfo
   {
      glm::mat4 invProj;
      glm::mat4 invView;
      glm::vec4 viewPos;
      glm::vec4 lightDir;
   } viewInfo;

   // This shouldn't be duplicated for every fog component
   BufferHandle viewInfoBuffer;

   struct Parameters
   {
      float time     = 0.0f;
      float a        = 0.0f;
      float b        = 0.0f;
      float startFog = 0.0f;
      float endFog   = 10000.0f;
   } params;
};
}
