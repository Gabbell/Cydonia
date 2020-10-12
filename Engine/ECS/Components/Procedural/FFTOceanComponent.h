#pragma once

#include <ECS/Components/BaseComponent.h>
#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
class FFTOceanComponent : public BaseComponent
{
  public:
   FFTOceanComponent() = default;
   FFTOceanComponent(
       uint32_t resolution,
       uint32_t horizontalDimension,
       float amplitude,
       float windSpeed,
       float windDirX,
       float windDirZ );
   COPIABLE( FFTOceanComponent );
   virtual ~FFTOceanComponent();

   static constexpr ComponentType TYPE = ComponentType::OCEAN;

   // Properties used as a single push constant in the multiple shader passes
   struct Parameters
   {
      uint32_t resolution          = 0;  // N - Phillips spectrum resolution
      uint32_t horizontalDimension = 0;  // L - Horizontal dimension of the patch
      uint32_t pingpong            = 0;  // Which texture should be used during the FFT operations
      uint32_t direction           = 0;  // 0-Horizontal 1-Vertical butterfly operation
      uint32_t stage               = 0;  // Which stage of the butterfly operation are we at (log2N)
      uint32_t componentMask       = 0;  // Used in displacement computation for RGB/XYZ masking
      float amplitude              = 0.0f;  // A
      float gravity                = 0.0f;  // Usually 9.8m/s^2
      float windSpeed              = 0.0f;
      float windDirX               = 0.0f;
      float windDirZ               = 0.0f;
      float time                   = 0.0f;
   } parameters;

   // Time-indepdendent textures (precomputed)
   // These textures are sampled each frame and are constant across time until one of the properties
   // for pre-computed textures below is changed.
   TextureHandle spectrum1;         // ~h0(k) - Phillips spectrum
   TextureHandle spectrum2;         // ~h0(-k) - Phillips spectrum
   TextureHandle butterflyTexture;  // twiddle indices
   BufferHandle bitReversedIndices;

   // Time-dependent textures
   // These textures are computed every frame.
   TextureHandle fourierComponentsX;  //
   TextureHandle fourierComponentsY;  // ~h(k,t)
   TextureHandle fourierComponentsZ;  //

   TextureHandle pingpongTex;

   float modulationY = 1.0f;  // Modulations applied when rendering the heightmap
   float modulationX = 1.0f;
   float modulationZ = 1.0f;

   bool needsUpdate       = true;
   bool resolutionChanged = true;
};
}
