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
      SHADOW,
      DEPTH
   };

   void resize( uint32_t width, uint32_t height );
   void bind( CmdListHandle cmdList ) const;

  private:
   void _destroy();

   TextureHandle m_albedo;
   TextureHandle m_normal;  // In worldspace
   TextureHandle m_shadow;  // Shadow mask
};
}