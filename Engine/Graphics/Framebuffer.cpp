#include <Graphics/Framebuffer.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void Framebuffer::resize( uint32_t width, uint32_t height )
{
   m_width  = width;
   m_height = height;
}

void Framebuffer::attach(
    uint32_t idx,
    TextureHandle texture,
    Access nextAccess,
    const ClearValue& clearValue )
{
   CYD_ASSERT(
       m_targets[idx].texture == 0 &&
       "Detach a previously bound target before attaching a new one" );

   m_targets[idx].texture    = texture;
   m_targets[idx].nextAccess = nextAccess;
   m_targets[idx].clearValue = clearValue;
}

void Framebuffer::replace(
    uint32_t idx,
    TextureHandle texture,
    Access nextAccess,
    const ClearValue& clearValue )
{
   detach( idx );
   attach( idx, texture, nextAccess, clearValue );
}

void Framebuffer::detach( uint32_t idx ) { m_targets[idx].texture = {}; }

void Framebuffer::bind( CmdListHandle cmdList, uint32_t idx, uint32_t binding, uint32_t set ) const
{
   if( m_targets[idx].texture )
   {
      GRIS::BindTexture( cmdList, m_targets[idx].texture, binding, set );
   }
}

bool Framebuffer::isValid() const { return m_width > 0 && m_height > 0 && !m_targets.empty(); }

void Framebuffer::setToClear( bool shouldClear )
{
   for( RenderTarget& rt : m_targets )
   {
      rt.shouldClear = shouldClear;
   }
}

void Framebuffer::setToClear( uint32_t idx, bool shouldClear )
{
   m_targets[idx].shouldClear = shouldClear;
}
}