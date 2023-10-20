#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    const Noise::ShaderParams& params )
    : type( noiseType ), width( width ), height( height ), params( params )
{
}

ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    float speed )
    : type( noiseType ), width( width ), height( height ), speed( speed )
{
}

ProceduralDisplacementComponent::~ProceduralDisplacementComponent()
{
   GRIS::DestroyTexture( texture );
}
}
