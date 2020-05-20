#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Handles/Handle.h>

namespace CYD
{
class FFTOceanComponent : public BaseComponent
{
  public:
   FFTOceanComponent() = default;
   COPIABLE( FFTOceanComponent )
   virtual ~FFTOceanComponent();

   bool init(
       uint32_t resolution,
       uint32_t horizontalDimension,
       uint32_t A,
       float windSpeed,
       float windDirX,
       float windDirZ );

   void uninit() override;

   static constexpr ComponentType TYPE = ComponentType::OCEAN;

   // Time-indepdendent textures (precomputed)
   // These textures are sampled each frame and are constant across time until one of the properties
   // for pre-computed textures below is changed.
   TextureHandle spectrum1;         // ~h0(k) - Phillips spectrum
   TextureHandle spectrum2;         // ~h0(-k) - Phillips spectrum
   TextureHandle butterflyTexture;  // twiddle indices

   // Properties for pre-computed textures
   struct SpectraGenInfo
   {
      uint32_t resolution          = 0;  // N - Phillips spectrum resolution
      uint32_t horizontalDimension = 0;  // L - Horizontal dimension of the patch
      uint32_t A                   = 0;
      float gravity                = 0.0f;
      float windSpeed              = 0.0f;
      float windDirX               = 0.0f;
      float windDirZ               = 0.0f;
   } spectraGenInfo;

   // Time-dependent textures
   // These textures are computed every frame.
   TextureHandle fourierComponents;  // ~h(k,t)
   TextureHandle heightField;        // h(x,t)

   float heightModulation = 1.0f;  // Height modulation applied when rendering the heightmap

   bool needsUpdate = true;
};
}
