#pragma once

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
struct PipelineInfo;

namespace GRIS
{
void NamedBufferBinding(
    CmdListHandle cmdList,
    BufferHandle buffer,
    std::string_view name,
    const PipelineInfo& pipInfo,
    uint32_t offset = 0,
    uint32_t range  = 0 );

void NamedTextureBinding(
    CmdListHandle cmdList,
    TextureHandle texture,
    std::string_view name,
    const PipelineInfo& pipInfo );

void NamedUpdateConstantBuffer(
    CmdListHandle cmdList,
    std::string_view name,
    const void* pData,
    const PipelineInfo& pipInfo );
}
}
