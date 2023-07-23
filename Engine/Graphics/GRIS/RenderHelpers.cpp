#pragma once

#include <Graphics/GRIS/RenderHelpers.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/PipelineInfos.h>

namespace CYD::GRIS
{
void NamedBufferBinding(
    CmdListHandle cmdList,
    BufferHandle buffer,
    std::string_view name,
    const PipelineInfo& pipInfo,
    uint32_t offset,
    uint32_t range )
{
   if( const FlatShaderBinding res = pipInfo.findBinding( buffer, name ); res.valid )
   {
      GRIS::BindUniformBuffer( cmdList, buffer, res.binding, res.set, offset, range );
   }
   else
   {
      CYD_ASSERT( !"Could not find named buffer" );
   }
}

void NamedTextureBinding(
    CmdListHandle cmdList,
    TextureHandle texture,
    std::string_view name,
    const PipelineInfo& pipInfo )
{
   if( const FlatShaderBinding res = pipInfo.findBinding( texture, name ); res.valid )
   {
      GRIS::BindTexture( cmdList, texture, res.binding, res.set );
   }
   else
   {
      CYD_ASSERT( !"Could not find named texture" );
   }
}

void NamedUpdateConstantBuffer(
    CmdListHandle cmdList,
    std::string_view name,
    const void* pData,
    const PipelineInfo& pipInfo )
{
   if( const PushConstantRange* range = pipInfo.findPushConstant( name ) )
   {
      GRIS::UpdateConstantBuffer( cmdList, range->stages, range->offset, range->size, pData );
   }
   else
   {
      CYD_ASSERT( !"Could not find named push constant" );
   }
}
}
