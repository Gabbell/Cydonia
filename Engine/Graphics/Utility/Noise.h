#pragma once

#include <Graphics/GraphicsTypes.h>

#include <Graphics/StaticPipelines.h>

#include <string_view>

namespace CYD
{
namespace Noise
{
enum class Type : uint8_t
{
   WHITE_NOISE,
   SIMPLEX_NOISE,
   VORONOI_NOISE,
   BLUE_NOISE,
   DOMAIN_WARP,
   COUNT
};

std::string_view GetNoiseName( Type type );

struct ShaderParams
{
   ShaderParams( uint32_t seed = 1 ) : seed( static_cast<float>( seed ) ) {}

   float seed       = 1.0f;  // Helps with randomization
   float scale      = 1.0f;
   float amplitude  = 1.0f;
   float gain       = 1.0f;  // Scales UV walking step (from 0.0f to 1.0f)
   float frequency  = 1.0f;
   float lacunarity = 1.0f;  // Changes fine scaling of each octaves (greater than 1.0f)
   float exponent   = 1.0f;  // Increase or decrease difference between valleys and ridges
   uint32_t ridged  = false;
   uint32_t invert  = false;
   uint32_t octaves = 1;
};

void Initialize();
void Uninitialize();

float GenerateRandomSeed();

PipelineIndex GetPipeline( Type type );
}
}