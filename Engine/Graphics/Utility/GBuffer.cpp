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

   m_textures[ALBEDO] = GRIS::CreateTexture( texDesc );

   // Normal
   texDesc.name   = "GBuffer Normal";
   texDesc.format = PixelFormat::RGBA32F;
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   m_textures[NORMAL] = GRIS::CreateTexture( texDesc );

   // PBR
   texDesc.name   = "GBuffer PBR";
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   m_textures[PBR] = GRIS::CreateTexture( texDesc );

   // Shadow
   texDesc.name   = "GBuffer Shadow Mask";
   texDesc.format = PixelFormat::R32F;
   texDesc.usage  = ImageUsage::COLOR | ImageUsage::SAMPLED;

   m_textures[SHADOW] = GRIS::CreateTexture( texDesc );

   ClearValue colorClear;
   colorClear.color.f32[0] = 0.0f;
   colorClear.color.f32[1] = 0.0f;
   colorClear.color.f32[2] = 0.0f;
   colorClear.color.f32[3] = 1.0f;

   Framebuffer::resize( width, height );
   attach( ALBEDO, m_textures[ALBEDO], Access::FRAGMENT_SHADER_READ );
   attach( NORMAL, m_textures[NORMAL], Access::FRAGMENT_SHADER_READ );
   attach( PBR, m_textures[PBR], Access::FRAGMENT_SHADER_READ );
   attach( SHADOW, m_textures[SHADOW], Access::FRAGMENT_SHADER_READ );
}

void GBuffer::bind( CmdListHandle cmdList ) const
{
   // This could be more flexible if needed
   Framebuffer::bind( cmdList, ALBEDO, 0, 1 );
   Framebuffer::bind( cmdList, NORMAL, 1, 1 );
   Framebuffer::bind( cmdList, PBR, 2, 1 );
   Framebuffer::bind( cmdList, SHADOW, 3, 1 );
   Framebuffer::bind( cmdList, DEPTH, 4, 1 );
}

void GBuffer::bind( CmdListHandle cmdList, Index index, uint32_t binding ) const
{
   Framebuffer::bind( cmdList, index, binding );
}
void GBuffer::_destroy()
{
   GRIS::DestroyTexture( m_textures[ALBEDO] );
   GRIS::DestroyTexture( m_textures[NORMAL] );
   GRIS::DestroyTexture( m_textures[PBR] );
   GRIS::DestroyTexture( m_textures[SHADOW] );
   detach( ALBEDO );
   detach( NORMAL );
   detach( PBR );
   detach( SHADOW );
   detach( DEPTH );  // Depth is optional and external
}
}