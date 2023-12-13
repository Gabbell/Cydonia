#pragma once

#include <ECS/Components/BaseComponent.h>
#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/GraphicsTypes.h>

#include <Physics/PhysicsConstants.h>

namespace CYD
{
class FFTOceanComponent final : public BaseComponent
{
  public:
   struct Description
   {
      uint32_t resolution          = 512;
      uint32_t horizontalDimension = 2000;
      float amplitude              = 100.0f;
      float gravity                = PHYSICS::GRAV_ACCELERATION_CONSTANT;
      float windSpeed              = 100.0f;
      float windDirX               = 1.0f;
      float windDirZ               = 1.0f;
      float horizontalScale        = 1.0f;
      float verticalScale          = 1.0f;
   };

   FFTOceanComponent() = default;
   FFTOceanComponent( const Description& desc );
   COPIABLE( FFTOceanComponent );
   virtual ~FFTOceanComponent();

   static constexpr ComponentType TYPE = ComponentType::OCEAN;

   static constexpr float NORMAL_MAP_RESOLUTION_MULT = 1.0f;

   // Properties used as a single push constant in the multiple shader passes
   struct ShaderParameters
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
      float horizontalScale        = 1.0f;
      float verticalScale          = 1.0f;
      float time                   = 0.0f;
   } params;

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

   TextureHandle pingpongTex;  // TODO Part of texture cache

   // RGB = XYZ displacement,
   TextureHandle displacementMap;

   // RGB = XYZ normal, A = Folding (Jacobian Determinant)
   TextureHandle normalMap;

   bool needsUpdate       = true;  // If we need to update the pre-computed textures
   bool resolutionChanged = true;  // If we need to resize the textures
};
}
