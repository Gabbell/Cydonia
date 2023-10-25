#include <ECS/Components/Procedural/AtmosphereComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
AtmosphereComponent::AtmosphereComponent( const Description& description )
{
   params.mieScatteringCoefficient =
       glm::vec4( description.mieScatteringCoefficient, description.mieScatteringScale );

   params.mieAbsorptionCoefficient =
       glm::vec4( description.mieAbsorptionCoefficient, description.mieAbsorptionScale );

   params.rayleighScatteringCoefficient =
       glm::vec4( description.rayleighScatteringCoefficient, description.rayleighScatteringScale );

   params.absorptionCoefficient =
       glm::vec4( description.absorptionCoefficient, description.absorptionScale );

   params.heightFog = glm::vec4(
       description.heightFogHeight,
       description.heightFogFalloff,
       description.heightFogStrength,
       0.0f );

   params.groundAlbedo       = glm::vec4( description.groundAlbedo, 0.0f );
   params.groundRadiusMM     = description.groundRadiusMM;
   params.atmosphereRadiusMM = description.atmosphereRadiusMM;
   params.miePhase           = description.miePhase;
   params.rayleighHeight     = description.rayleighHeight;
   params.mieHeight          = description.mieHeight;
}

AtmosphereComponent::~AtmosphereComponent()
{
   GRIS::DestroyBuffer( viewInfoBuffer );
   GRIS::DestroyTexture( transmittanceLUT );
   GRIS::DestroyTexture( multipleScatteringLUT );
   GRIS::DestroyTexture( skyViewLUT );
   GRIS::DestroyTexture( aerialPerspectiveLUT );
}
}
