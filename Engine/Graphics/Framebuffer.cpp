#include <Graphics/Framebuffer.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void Framebuffer::resize( uint32_t width, uint32_t height, uint32_t layers )
{
   m_width  = width;
   m_height = height;
   m_layers = layers;
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

void Framebuffer::bindImage( CmdListHandle cmdList, uint32_t idx, uint32_t binding, uint32_t set )
    const
{
   if( m_targets[idx].texture )
   {
      GRIS::BindImage( cmdList, m_targets[idx].texture, binding, set );
   }
}

void Framebuffer::bindTexture( CmdListHandle cmdList, uint32_t idx, uint32_t binding, uint32_t set )
    const
{
   if( m_targets[idx].texture )
   {
      GRIS::BindTexture( cmdList, m_targets[idx].texture, binding, set );
   }
}

bool Framebuffer::isValid() const { return m_width > 0 && m_height > 0; }
void Framebuffer::setClearAll( bool shouldClear ) { m_clearAll = shouldClear; }
}