#pragma once

#include <Graphics/GraphicsTypes.h>

namespace vk
{
class Texture;
class CommandBuffer;
}

namespace vk::Barriers
{
void ImageMemory( const CommandBuffer* cmdBuffer, Texture* texture, CYD::ImageLayout targetLayout );
void Pipeline( const CommandBuffer* cmdBuffer, CYD::PipelineStageFlag sourceStage, CYD::PipelineStageFlag destStage );
}
