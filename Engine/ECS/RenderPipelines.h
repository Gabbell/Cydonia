#pragma once

#include <Handles/Handle.h>

#include <glm/glm.hpp>

namespace CYD
{
class RenderableComponent;

namespace RenderPipelines
{
bool Initialize();
}

// Pipeline with custom fragment shader
namespace CustomPipeline
{
void Render(
    CmdListHandle cmdList,
    const double deltaS,
    const RenderableComponent* renderable );
}

// Default pipeline
namespace DefaultPipeline
{
void Render(
    CmdListHandle cmdList,
    const glm::mat4& modelMatrix,
    const RenderableComponent* renderable );
}

// Pipeline with Phong lighting
namespace PhongPipeline
{
static constexpr uint32_t MATERIAL = 1;  // Material set
void Render(
    CmdListHandle cmdList,
    const glm::mat4& modelMatrix,
    const RenderableComponent* renderable );
}

// Pipeline with Physically Based Rendering (PBR) lighting
namespace PBRPipeline
{
static constexpr uint32_t MATERIAL = 1;  // Material set
void Render(
    CmdListHandle cmdList,
    const glm::mat4& modelMatrix,
    const RenderableComponent* renderable );
}
}