#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Common/PhysicsConstants.h>

#include <Graphics/RenderInterface.h>

namespace CYD
{
bool FFTOceanComponent::init(
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

   return true;
}

void FFTOceanComponent::uninit()
{
   GRIS::DestroyTexture( spectrum1 );
   GRIS::DestroyTexture( spectrum2 );
   GRIS::DestroyTexture( butterflyTexture );
   GRIS::DestroyBuffer( bitReversedIndices );

   GRIS::DestroyTexture( fourierComponentsY );
   GRIS::DestroyTexture( fourierComponentsX );
   GRIS::DestroyTexture( fourierComponentsZ );
}

FFTOceanComponent::~FFTOceanComponent() { FFTOceanComponent::uninit(); }
}
