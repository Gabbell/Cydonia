#include <Graphics/GRIS/TextureCache.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <unordered_map>

namespace CYD::GRIS::TextureCache
{
TextureHandle s_blackTex      = {};
TextureHandle s_whiteTex      = {};
TextureHandle s_pinkTex       = {};
TextureHandle s_depthTexArray = {};  // Used as default shadow maps

void Initialize()
{
   // Default 2D Textures
   TextureDescription texDesc;
   texDesc.width  = 1;
   texDesc.height = 1;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.stages = PipelineStage::FRAGMENT_STAGE;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   texDesc.name = "Default Black Texture";
   s_blackTex   = GRIS::CreateTexture( texDesc );

   texDesc.name = "Default White Texture";
   s_whiteTex   = GRIS::CreateTexture( texDesc );

   texDesc.name = "Default Pink Texture";
   s_pinkTex    = GRIS::CreateTexture( texDesc );

   // Default Depth 2D Texture Array
   TextureDescription depthTexArrayDesc;
   depthTexArrayDesc.width  = 1;
   depthTexArrayDesc.height = 1;
   depthTexArrayDesc.depth  = 1;
   depthTexArrayDesc.type   = ImageType::TEXTURE_2D_ARRAY;
   depthTexArrayDesc.format = PixelFormat::D32_SFLOAT;
   depthTexArrayDesc.stages = PipelineStage::FRAGMENT_STAGE;
   depthTexArrayDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   depthTexArrayDesc.name = "Default Depth Texture Array";
   s_depthTexArray        = GRIS::CreateTexture( depthTexArrayDesc );

   // Creating and clearing textures
   CmdListHandle cmdList = GRIS::CreateCommandList( QueueUsage::TRANSFER, "Initial TextureCache" );

   ClearValue clearVal = {};
   GRIS::ClearTexture( cmdList, s_blackTex, clearVal );

   clearVal.color.f32[0] = 1.0f;
   clearVal.color.f32[1] = 1.0f;
   clearVal.color.f32[2] = 1.0f;
   clearVal.color.f32[3] = 1.0f;
   GRIS::ClearTexture( cmdList, s_whiteTex, clearVal );

   clearVal.color.f32[0] = 1.0f;
   clearVal.color.f32[1] = 0.0f;
   clearVal.color.f32[2] = 1.0f;
   clearVal.color.f32[3] = 1.0f;
   GRIS::ClearTexture( cmdList, s_pinkTex, clearVal );

   clearVal.depthStencil.depth = 1.0f;
   clearVal.depthStencil.stencil = 0;
   GRIS::ClearTexture( cmdList, s_depthTexArray, clearVal );

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

void BindDepthTextureArray( CmdListHandle cmdList, uint32_t binding, uint32_t set )
{
   GRIS::BindTexture( cmdList, s_depthTexArray, binding, set );
}

TextureHandle GetBlackTexture() { return s_blackTex; }
TextureHandle GetWhiteTexture() { return s_whiteTex; }
TextureHandle GetPinkTexture() { return s_pinkTex; }
TextureHandle GetDepthTextureArray() { return s_depthTexArray; }
}