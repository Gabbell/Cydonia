#include <ECS/Components/Procedural/DisplacementComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
DisplacementComponent::DisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    const Noise::ShaderParams& params,
    bool generateNormals,
    float speed )
    : type( noiseType ),
      params( params ),
      width( width ),
      height( height ),
      speed( speed ),
      generateNormals( generateNormals )
{
}

DisplacementComponent::~DisplacementComponent()
{
   GRIS::DestroyTexture( displacementMap );
   GRIS::DestroyTexture( normalMap );
}
}
