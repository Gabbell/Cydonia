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
   struct Description
   {
      // These coefficients are per KM
      glm::vec3 mieScatteringCoefficient;
      glm::vec3 mieAbsorptionCoefficient;
      glm::vec3 rayleighScatteringCoefficient;
      glm::vec3 absorptionCoefficient;
      glm::vec3 groundAlbedo;
      float heightFogHeight;
      float heightFogFalloff;
      float heightFogStrength;
      float groundRadiusMM;
      float atmosphereRadiusMM;
      float miePhase;
      float mieHeight;
      float mieScatteringScale;
      float mieAbsorptionScale;
      float rayleighHeight;
      float rayleighScatteringScale;
      float absorptionScale;
   };

   AtmosphereComponent() = default;
   AtmosphereComponent( const Description& description );
   COPIABLE( AtmosphereComponent );
   virtual ~AtmosphereComponent();

   static constexpr ComponentType TYPE = ComponentType::ATMOSPHERE;

   struct ShaderParams
   {
      glm::vec4 mieScatteringCoefficient;
      glm::vec4 mieAbsorptionCoefficient;
      glm::vec4 rayleighScatteringCoefficient;
      glm::vec4 absorptionCoefficient;
      glm::vec4 groundAlbedo;
      glm::vec4 heightFog;
      float groundRadiusMM;
      float atmosphereRadiusMM;
      float miePhase;
      float rayleighHeight;
      float mieHeight;
      float nearClip;
      float farClip;
      float time;
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

   static constexpr uint32_t TRANSMITTANCE_LUT_WIDTH      = 256;
   static constexpr uint32_t TRANSMITTANCE_LUT_HEIGHT     = 64;
   static constexpr uint32_t MULTIPLE_SCATTERING_LUT_DIM  = 32;
   static constexpr uint32_t SKYVIEW_LUT_WIDTH            = 200;
   static constexpr uint32_t SKYVIEW_LUT_HEIGHT           = 200;
   static constexpr uint32_t AERIAL_PERSPECTIVE_LUT_DIM   = 32;
   static constexpr uint32_t AERIAL_PERSPECTIVE_LUT_DEPTH = 32;

   bool needsUpdate = true;

   // Can be precomputed with a set of atmospheric parameters
   TextureHandle transmittanceLUT;
   TextureHandle multipleScatteringLUT;

   // Need to be regenerated when position or direction changes
   TextureHandle skyViewLUT;
   TextureHandle aerialPerspectiveLUT;
};
}
