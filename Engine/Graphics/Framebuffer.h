#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class Framebuffer
{
  public:
   Framebuffer() = default;
   Framebuffer( uint32_t width, uint32_t height ) : m_width( width ), m_height( height ) {}
   COPIABLE( Framebuffer );
   ~Framebuffer() = default;

   enum Index
   {
      COLOR,
      DEPTH,
      COUNT
   };

   using Targets = std::array<TextureHandle, Index::COUNT>;

   void attach( Index idx, TextureHandle texture ) { m_targets[idx] = texture; }

   void detach( Index idx ) { m_targets[idx] = {}; }

   uint32_t getWidth() const { return m_width; }
   uint32_t getHeight() const { return m_height; }

   const Targets& getTargets() const { return m_targets; }
   TextureHandle getTarget( Index idx ) const { return m_targets[idx]; }

  private:
   uint32_t m_width  = 0;
   uint32_t m_height = 0;

   Targets m_targets;
};
}