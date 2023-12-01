#pragma once

#include <Common/Include.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
class GBuffer : public Framebuffer
{
  public:
   GBuffer() = default;
   MOVABLE( GBuffer );
   ~GBuffer();

   enum Index
   {
      ALBEDO,
      NORMAL,
      PBR,  // R = Roughness, Y = Metalness, Z = AO, W = Unused
      SHADOW,
      DEPTH,
      COUNT = DEPTH
   };

   void resize( uint32_t width, uint32_t height );
   void bind( CmdListHandle cmdList ) const;
   void bind( CmdListHandle cmdList, Index index, uint32_t binding ) const;

  private:
   void _destroy();

   TextureHandle m_textures[Index::COUNT];
};
}