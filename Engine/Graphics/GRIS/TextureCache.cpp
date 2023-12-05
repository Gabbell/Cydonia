#include <Graphics/GRIS/TextureCache.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD::GRIS::TextureCache
{
TextureHandle s_blackTex = {};
TextureHandle s_whiteTex = {};
TextureHandle s_pinkTex  = {};

void Initialize()
{
   TextureDescription texDesc;
   texDesc.width  = 1;
   texDesc.height = 1;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.stages = PipelineStage::FRAGMENT_STAGE;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   texDesc.name = "Default Black Texture";
   s_blackTex   = GRIS::CreateTexture( texDesc );

   texDesc.name = "Default White Texture";
   s_whiteTex   = GRIS::CreateTexture( texDesc );

   texDesc.name = "Default Pink Texture";
   s_pinkTex    = GRIS::CreateTexture( texDesc );

   CmdListHandle cmdList = GRIS::CreateCommandList( QueueUsage::TRANSFER, "Initial TextureCache" );

   ClearValue clearVal = {};
   GRIS::ClearTexture( cmdList, s_blackTex, clearVal );

   memset( clearVal.color.u32, 0xFFFFFFFF, sizeof( clearVal.color.u32 ) );
   GRIS::ClearTexture( cmdList, s_whiteTex, clearVal );

   clearVal.color.f32[0] = 1.0f;
   clearVal.color.f32[1] = 0.0f;
   clearVal.color.f32[2] = 1.0f;
   GRIS::ClearTexture( cmdList, s_pinkTex, clearVal );

   GRIS::SubmitCommandList( cmdList );
   GRIS::WaitOnCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );
}

void Uninitialize()
{
   GRIS::DestroyTexture( s_blackTex );
   GRIS::DestroyTexture( s_whiteTex );
   GRIS::DestroyTexture( s_pinkTex );
}

void BindBlackTexture( CmdListHandle cmdList, uint32_t binding, uint32_t set )
{
   GRIS::BindTexture( cmdList, s_blackTex, binding, set );
}

void BindWhiteTexture( CmdListHandle cmdList, uint32_t binding, uint32_t set )
{
   GRIS::BindTexture( cmdList, s_whiteTex, binding, set );
}

void BindPinkTexture( CmdListHandle cmdList, uint32_t binding, uint32_t set )
{
   GRIS::BindTexture( cmdList, s_pinkTex, binding, set );
}
}