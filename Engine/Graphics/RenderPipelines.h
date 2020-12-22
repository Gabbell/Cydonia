#pragma once

#include <cstdint>
#include <string_view>

namespace CYD
{
struct PipelineInfo;
}

namespace CYD::RenderPipelines
{
bool Initialize();
void Uninitialize();

const PipelineInfo* Get( std::string_view pipName );
}