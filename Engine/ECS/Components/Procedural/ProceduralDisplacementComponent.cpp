#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    const Noise::ShaderParams& params )
    : type( noiseType ), params( params ), width( width ), height( height )
{
}

ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    float speed )
    : type( noiseType ), speed( speed ), width( width ), height( height )
{
}

ProceduralDisplacementComponent::~ProceduralDisplacementComponent()
{
   GRIS::DestroyTexture( texture );
}
}
