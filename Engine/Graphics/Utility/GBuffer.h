#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
class GBuffer
{
   GBuffer() = default;
   MOVABLE( GBuffer );
   ~GBuffer();

  private:
   TextureHandle position;  // In worldspace
   TextureHandle albedo;
   TextureHandle normals;  // In worldspace
   TextureHandle depth;
};
}