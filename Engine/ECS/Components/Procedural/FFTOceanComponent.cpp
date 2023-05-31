#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Physics/PhysicsConstants.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
FFTOceanComponent::FFTOceanComponent(
    uint32_t resolution,
    uint32_t horizontalDimension,
    float amplitude,
    float windSpeed,
    float windDirX,
    float windDirZ )
{
   params.resolution          = resolution;
   params.horizontalDimension = horizontalDimension;
   params.amplitude           = amplitude;
   params.gravity             = PHYSICS::GRAV_ACCELERATION_CONSTANT;
   params.windSpeed           = windSpeed;
   params.windDirX            = windDirX;
   params.windDirZ            = windDirZ;
}

FFTOceanComponent::~FFTOceanComponent()
{
   GRIS::DestroyTexture( spectrum1 );
   GRIS::DestroyTexture( spectrum2 );
   GRIS::DestroyTexture( butterflyTexture );
   GRIS::DestroyBuffer( bitReversedIndices );

   GRIS::DestroyTexture( fourierComponentsY );
   GRIS::DestroyTexture( fourierComponentsX );
   GRIS::DestroyTexture( fourierComponentsZ );

   GRIS::DestroyTexture( pingpongTex );
}
}
