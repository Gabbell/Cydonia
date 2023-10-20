#include <ECS/Components/Procedural/AtmosphereComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
AtmosphereComponent::AtmosphereComponent( float phaseScale, float heightFogA, float heightFogB )
{
   params.phaseScale = phaseScale;
   params.heightFogA = heightFogA;
   params.heightFogB = heightFogB;
}

AtmosphereComponent::~AtmosphereComponent()
{
   GRIS::DestroyBuffer( viewInfoBuffer );
   GRIS::DestroyTexture( transmittanceLUT );
   GRIS::DestroyTexture( multipleScatteringLUT );
   GRIS::DestroyTexture( skyViewLUT );
}
}
