#pragma once

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
namespace GRIS::TextureCache
{
void Initialize();
void Uninitialize();

void BindBlackTexture( CmdListHandle cmdList, uint32_t binding, uint32_t set );
void BindWhiteTexture( CmdListHandle cmdList, uint32_t binding, uint32_t set );
void BindPinkTexture( CmdListHandle cmdList, uint32_t binding, uint32_t set );
void BindDepthTextureArray( CmdListHandle cmdList, uint32_t binding, uint32_t set );

TextureHandle GetBlackTexture();
TextureHandle GetWhiteTexture();
TextureHandle GetPinkTexture();
TextureHandle GetDepthTextureArray();
}
}