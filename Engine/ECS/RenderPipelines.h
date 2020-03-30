#pragma once

#include <Handles/Handle.h>

#include <glm/glm.hpp>

namespace cyd
{
class RenderableComponent;

namespace PhongPipeline
{
static constexpr uint32_t MATERIAL = 1;  // Material set

void render( CmdListHandle cmdList, const glm::mat4& model, const RenderableComponent* renderable );
}

namespace PBRPipeline
{
static constexpr uint32_t MATERIAL = 1;  // Material set
void render( CmdListHandle cmdList, const glm::mat4& model, const RenderableComponent* renderable );
}
}