#pragma once

#include <cstdint>

namespace CYD
{
struct PipelineInfo;
}

namespace CYD::StaticPipelines
{
bool Initialize();
void Uninitialize();

// Keep this in SYNC with the indices in Pipelines.json
enum class Type : uint8_t
{
   DEFAULT      = 0,
   DEFAULT_TEX  = 1,
   DISPLACEMENT = 2,
   PHONG        = 3,
   PHONG_TEX    = 4,
   PBR          = 5,
   SKYBOX       = 6,

   COUNT
};

const PipelineInfo* Get( StaticPipelines::Type type );
}