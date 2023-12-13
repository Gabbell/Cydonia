#include <Graphics/Utility/Noise.h>

#include <chrono>
#include <random>

namespace CYD::Noise
{
static bool s_initialized = false;

PipelineIndex s_noisePipelines[] = {
    INVALID_PIPELINE_IDX,
    INVALID_PIPELINE_IDX,
    INVALID_PIPELINE_IDX,
    INVALID_PIPELINE_IDX,
    INVALID_PIPELINE_IDX };

const char* s_noiseNames[] =
    { "WHITE_NOISE", "SIMPLEX_NOISE", "VORONOI_NOISE", "BLUE_NOISE", "DOMAIN_WARPED_NOISE" };

static_assert( ARRSIZE( s_noisePipelines ) == static_cast<size_t>( Type::COUNT ) );
static_assert( ARRSIZE( s_noiseNames ) == static_cast<size_t>( Type::COUNT ) );

std::string_view GetNoiseName( Type type ) { return s_noiseNames[UNDERLYING( type )]; }

void Initialize()
{
   if( !s_initialized )
   {
      for( uint32_t i = 0; i < static_cast<uint32_t>( Noise::Type::COUNT ); ++i )
      {
         s_noisePipelines[i] = StaticPipelines::FindByName( s_noiseNames[i] );
      }

      s_initialized = true;
   }
}

void Uninitialize()
{
   // Remove pipelines
}

float GenerateRandomSeed()
{
   uint32_t seed =
       static_cast<uint32_t>( std::chrono::system_clock::now().time_since_epoch().count() );
   std::mt19937 generator( seed );
   return static_cast<float>( generator() ) / static_cast<float>( generator.max() );
}

PipelineIndex GetPipeline( Type type )
{
   const uint32_t typeIdx = static_cast<uint32_t>( type );
   return s_noisePipelines[typeIdx];
}
}