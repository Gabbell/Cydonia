#pragma once

#include <Handles/Handle.h>

#include <glm/glm.hpp>

namespace CYD
{
class MeshComponent;
class RenderableComponent;

namespace RenderPipelines
{
bool Initialize();
void Prepare(
    CmdListHandle cmdList,
    const double deltaS,
    const glm::mat4& modelMatrix,
    const RenderableComponent* renderable );
}
}