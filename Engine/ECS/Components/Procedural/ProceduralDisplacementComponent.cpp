#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    uint32_t seed )
    : type( noiseType ), width( width ), height( height ), params( seed )
{
}

ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    float speed )
    : type( noiseType ), width( width ), height( height ), timeMultiplier( speed )
{
}

ProceduralDisplacementComponent::~ProceduralDisplacementComponent()
{
   GRIS::DestroyTexture( texture );
}
}
