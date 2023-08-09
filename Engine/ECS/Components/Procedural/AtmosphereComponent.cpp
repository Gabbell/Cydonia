#include <ECS/Components/Procedural/AtmosphereComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
AtmosphereComponent::~AtmosphereComponent()
{
   GRIS::DestroyBuffer( viewInfoBuffer );
   GRIS::DestroyTexture( transmittanceLUT );
   GRIS::DestroyTexture( multipleScatteringLUT );
   GRIS::DestroyTexture( skyViewLUT );
}
}
