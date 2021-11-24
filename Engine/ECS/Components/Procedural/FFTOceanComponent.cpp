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
   parameters.resolution          = resolution;
   parameters.horizontalDimension = horizontalDimension;
   parameters.amplitude           = amplitude;
   parameters.gravity             = PHYSICS::GRAV_ACCELERATION_CONSTANT;
   parameters.windSpeed           = windSpeed;
   parameters.windDirX            = windDirX;
   parameters.windDirZ            = windDirZ;
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
}
}
