#include "Graphics/Utility/GBuffer.h"

#include "Common/Assert.h"

#include "Graphics/GRIS/RenderInterface.h"

namespace CYD
{
GBuffer::~GBuffer() { _destroy(); }

void GBuffer::resize( uint32_t width, uint32_t height )
{
   CYD_ASSERT( width > 0 && height > 0 );

   if( width == getWidth() && height == getHeight() )
   {
      return;
   }

   _destroy();

   TextureDescription texDesc;
   texDesc.width  = width;
   texDesc.height = height;
   texDesc.stages = PipelineStage::FRAGMENT_STAGE;
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   // Albedo
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.name   = "GBuffer Albedo";
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   m_albedo = GRIS::CreateTexture( texDesc );

   // Normal
   texDesc.name   = "GBuffer Normal";
   texDesc.format = PixelFormat::RGBA32F;
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   m_normal = GRIS::CreateTexture( texDesc );

   // Shadow
   texDesc.name   = "GBuffer Shadow";
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   m_shadow = GRIS::CreateTexture( texDesc );

   ClearValue colorClear;
   colorClear.color.f32[0] = 0.0f;
   colorClear.color.f32[1] = 0.0f;
   colorClear.color.f32[2] = 0.0f;
   colorClear.color.f32[3] = 1.0f;

   Framebuffer::resize( width, height );
   attach( ALBEDO, m_albedo, Access::FRAGMENT_SHADER_READ );
   attach( NORMAL, m_normal, Access::FRAGMENT_SHADER_READ );
   attach( SHADOW, m_shadow, Access::FRAGMENT_SHADER_READ );
}

void GBuffer::bind( CmdListHandle cmdList ) const
{
   // This could be more flexible if needed
   Framebuffer::bind( cmdList, ALBEDO, 0 );
   Framebuffer::bind( cmdList, NORMAL, 1 );
   Framebuffer::bind( cmdList, SHADOW, 2 );
   Framebuffer::bind( cmdList, DEPTH, 3 );
}

void GBuffer::_destroy()
{
   GRIS::DestroyTexture( m_albedo );
   GRIS::DestroyTexture( m_normal );
   GRIS::DestroyTexture( m_shadow );
   detach( ALBEDO );
   detach( NORMAL );
   detach( SHADOW );
   detach( DEPTH );
}
}