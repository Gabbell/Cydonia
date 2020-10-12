#include <Graphics/PipelineInfos.h>

namespace CYD
{
PipelineInfo::PipelineInfo()  = default;
PipelineInfo::~PipelineInfo() = default;

GraphicsPipelineInfo::GraphicsPipelineInfo() : PipelineInfo( PipelineType::GRAPHICS ) {}

// TODO Also compare shader constants
bool GraphicsPipelineInfo::operator==( const GraphicsPipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && drawPrim == other.drawPrim &&
          polyMode == other.polyMode && extent == other.extent && shaders == other.shaders;
}
GraphicsPipelineInfo::~GraphicsPipelineInfo() = default;

ComputePipelineInfo::ComputePipelineInfo() : PipelineInfo( PipelineType::COMPUTE ) {}

bool ComputePipelineInfo::operator==( const ComputePipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && shader == other.shader;
}
ComputePipelineInfo::~ComputePipelineInfo() = default;
}