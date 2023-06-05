#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <Graphics/Utility/Noise.h>

namespace CYD
{
ProceduralDisplacementComponent::ProceduralDisplacementComponent(
    Noise::Type noiseType,
    uint32_t width,
    uint32_t height,
    uint32_t seed )
{
   type          = noiseType;
   params.width  = width;
   params.height = height;

   // If seed is not specified, generate random
   if( seed == 0 )
   {
      params.seed = Noise::GenerateRandomSeed();
   }
   else
   {
      params.seed = static_cast<float>( seed ) / UINT_MAX;
   }
}

ProceduralDisplacementComponent::~ProceduralDisplacementComponent()
{
   GRIS::DestroyTexture( texture );
}
}
