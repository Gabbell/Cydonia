#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Common/PhysicsConstants.h>

#include <Graphics/RenderInterface.h>

namespace CYD
{
bool FFTOceanComponent::init(
    uint32_t resolution,
    uint32_t horizontalDimension,
    uint32_t A,
    float windSpeed,
    float windDirX,
    float windDirZ )
{
   spectraGenInfo.resolution          = resolution;
   spectraGenInfo.horizontalDimension = horizontalDimension;
   spectraGenInfo.A                   = A;
   spectraGenInfo.gravity             = PHYSICS::GRAV_ACCELERATION_CONSTANT;
   spectraGenInfo.windSpeed           = windSpeed;
   spectraGenInfo.windDirX            = windDirX;
   spectraGenInfo.windDirZ            = windDirZ;

   return true;
}

void FFTOceanComponent::uninit()
{
   GRIS::DestroyTexture( spectrum1 );
   GRIS::DestroyTexture( spectrum2 );
   GRIS::DestroyTexture( butterflyTexture );
   GRIS::DestroyTexture( fourierComponents );
   GRIS::DestroyTexture( heightField );
}

FFTOceanComponent::~FFTOceanComponent() { FFTOceanComponent::uninit(); }
}
