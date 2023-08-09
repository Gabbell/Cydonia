#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
FFTOceanComponent::FFTOceanComponent( const Description& desc )
{
   params.resolution          = desc.resolution;
   params.horizontalDimension = desc.horizontalDimension;
   params.amplitude           = desc.amplitude;
   params.gravity             = desc.gravity;
   params.windSpeed           = desc.windSpeed;
   params.windDirX            = desc.windDirX;
   params.windDirZ            = desc.windDirZ;
   params.horizontalScale     = desc.horizontalScale;
   params.verticalScale       = desc.verticalScale;
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

   GRIS::DestroyTexture( displacementMap );
}
}
