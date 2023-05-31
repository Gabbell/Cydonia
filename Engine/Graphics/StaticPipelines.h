#pragma once

#include <Graphics/GraphicsTypes.h>

#include <cstdint>
#include <string_view>

namespace CYD
{
struct PipelineInfo;
}

namespace CYD
{
namespace StaticPipelines
{
bool Initialize();
void Uninitialize();

PipelineIndex FindByName( std::string_view pipName );
const PipelineInfo* Get( PipelineIndex index );
}
}