#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

namespace CYD::ShadowMapping
{
constexpr uint32_t MAX_RESOLUTION = 2048;
constexpr uint32_t MAX_CASCADES   = 4;

SamplerInfo GetSampler();

TextureDescription GetFilterableTextureDescription( uint32_t resolution, uint32_t numCascades = 1 );
TextureDescription GetDepthTextureDescription( uint32_t resolution, uint32_t numCascades = 1 );
}