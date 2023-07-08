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

   enum class Index
   {
      COLOR,
      DEPTH,
      COUNT
   };

   using Targets = std::array<TextureHandle, static_cast<uint32_t>( Index::COUNT )>;

   void attach( Index idx, TextureHandle texture )
   {
      const uint32_t attachmentIndex = static_cast<uint32_t>( idx );
      m_targets[attachmentIndex]     = texture;
   }

   void detach( Index idx )
   {
      const uint32_t attachmentIndex = static_cast<uint32_t>( idx );
      m_targets[attachmentIndex]     = {};
   }

   uint32_t getWidth() const { return m_width; }
   uint32_t getHeight() const { return m_height; }

   const Targets& getTargets() const { return m_targets; }
   TextureHandle getTarget( Index idx ) const { return m_targets[static_cast<uint32_t>( idx )]; }

  private:
   uint32_t m_width  = 0;
   uint32_t m_height = 0;

   Targets m_targets;
};
}