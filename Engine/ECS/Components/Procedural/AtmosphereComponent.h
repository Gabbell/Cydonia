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
class AtmosphereComponent final : public BaseComponent
{
  public:
   AtmosphereComponent() = default;
   COPIABLE( AtmosphereComponent );
   virtual ~AtmosphereComponent();

   static constexpr ComponentType TYPE = ComponentType::ATMOSPHERE;

   struct Parameters
   {
      float groundRadiusMM     = 0.0f;
      float atmosphereRadiusMM = 0.0f;
   } params;

   struct ViewInfo
   {
      glm::mat4 invProj;
      glm::mat4 invView;
      glm::vec4 viewPos;
      glm::vec4 lightDir;
   } viewInfo;

   // This shouldn't be duplicated for every atmosphere component
   BufferHandle viewInfoBuffer;

   static constexpr uint32_t TRANSMITTANCE_LUT_WIDTH     = 256;
   static constexpr uint32_t TRANSMITTANCE_LUT_HEIGHT    = 64;
   static constexpr uint32_t MULTIPLE_SCATTERING_LUT_DIM = 32;
   static constexpr uint32_t SKYVIEW_LUT_WIDTH           = 200;
   static constexpr uint32_t SKYVIEW_LUT_HEIGHT          = 200;

   bool needsUpdate = true;

   TextureHandle transmittanceLUT;
   TextureHandle multipleScatteringLUT;

   // Depends on view position and direction
   TextureHandle skyViewLUT;
};
}
