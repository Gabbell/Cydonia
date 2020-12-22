#include <Graphics/GRIS/RenderInterface.h>

#include <Common/Assert.h>

#include <Graphics/RenderPipelines.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/GRIS/Backends/VKRenderBackend.h>

#include <cstdio>

namespace CYD::GRIS
{
static RenderBackend* b = nullptr;

// =================================================================================================
// Initialization
//
bool InitRenderBackend( API api, const Window& window )
{
   RenderPipelines::Initialize();

   delete b;

   switch( api )
   {
      case VK:
         printf( "======= Initializing Vulkan Rendering Backend =======\n" );
         b = new VKRenderBackend( window );
         return true;
      case D3D12:
         printf( "======= DirectX12 Rendering Backend Not Yet Implemented =======\n" );
         return false;
      case D3D11:
         printf( "======= DirectX11 Rendering Backend Not Yet Implemented =======\n" );
         return false;
      case GL:
         printf( "======= OpenGL Rendering Backend Not Yet Implemented =======\n" );
         return false;
      case MTL:
         printf( "======= Metal Rendering Backend Not Yet Implemented =======\n" );
         return false;
   }

   return false;
}

void UninitRenderBackend()
{
   printf( "======= Reports of my death have been greatly exaggerated =======\n" );

   RenderPipelines::Uninitialize();

   delete b;
}

void RenderBackendCleanup() { b->cleanup(); }

// =================================================================================================
// Command Buffers/Lists
//
CmdListHandle CreateCommandList( QueueUsageFlag usage, bool presentable )
{
   return b->createCommandList( usage, presentable );
}

void StartRecordingCommandList( CmdListHandle cmdList ) { b->startRecordingCommandList( cmdList ); }
void EndRecordingCommandList( CmdListHandle cmdList ) { b->endRecordingCommandList( cmdList ); }
void SubmitCommandList( CmdListHandle cmdList ) { b->submitCommandList( cmdList ); }
void ResetCommandList( CmdListHandle cmdList ) { b->resetCommandList( cmdList ); }
void WaitOnCommandList( CmdListHandle cmdList ) { b->waitOnCommandList( cmdList ); }
void DestroyCommandList( CmdListHandle cmdList ) { b->destroyCommandList( cmdList ); }

// =================================================================================================
// Pipeline Specification
//
void SetViewport( CmdListHandle cmdList, const Viewport& viewport )
{
   b->setViewport( cmdList, viewport );
}

void SetScissor( CmdListHandle cmdList, const Rectangle& scissor )
{
   b->setScissor( cmdList, scissor );
}

void BindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void BindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void BindPipeline( CmdListHandle cmdList, std::string_view pipName )
{
   const PipelineInfo* pPipInfo = RenderPipelines::Get( pipName );

   if( pPipInfo )
   {
      switch( pPipInfo->type )
      {
         case PipelineType::GRAPHICS:
            b->bindPipeline( cmdList, *static_cast<const GraphicsPipelineInfo*>( pPipInfo ) );
            break;
         case PipelineType::COMPUTE:
            b->bindPipeline( cmdList, *static_cast<const ComputePipelineInfo*>( pPipInfo ) );
            break;
      }
   }
}

void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   b->bindVertexBuffer( cmdList, bufferHandle );
}

template <>
void BindIndexBuffer<uint16_t>( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
{
   b->bindIndexBuffer( cmdList, bufferHandle, IndexType::UNSIGNED_INT16 );
}

template <>
void BindIndexBuffer<uint32_t>( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
{
   b->bindIndexBuffer( cmdList, bufferHandle, IndexType::UNSIGNED_INT32 );
}

void BindTexture( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding )
{
   b->bindTexture( cmdList, texHandle, set, binding );
}

void BindTexture( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name )
{
   b->bindTexture( cmdList, texHandle, name );
}

void BindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding )
{
   b->bindImage( cmdList, texHandle, set, binding );
}

void BindBuffer( CmdListHandle cmdList, BufferHandle bufferHandle, uint32_t set, uint32_t binding )
{
   b->bindBuffer( cmdList, bufferHandle, set, binding );
}

void BindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t set,
    uint32_t binding )
{
   b->bindUniformBuffer( cmdList, bufferHandle, set, binding );
}

void UpdateConstantBuffer(
    CmdListHandle cmdList,
    ShaderStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData )
{
   b->updateConstantBuffer( cmdList, stages, offset, size, pData );
}

// =================================================================================================
// Resources
//
TextureHandle CreateTexture( CmdListHandle transferList, const TextureDescription& desc )
{
   return b->createTexture( transferList, desc );
}

TextureHandle
CreateTexture( CmdListHandle transferList, const TextureDescription& desc, const std::string& path )
{
   return b->createTexture( transferList, desc, path );
}

TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    const std::vector<std::string>& paths )
{
   return b->createTexture( transferList, desc, paths );
}

TextureHandle
CreateTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels )
{
   return b->createTexture( transferList, desc, pTexels );
}

VertexBufferHandle CreateVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices )
{
   return b->createVertexBuffer( transferList, count, stride, pVertices );
}

IndexBufferHandle
CreateIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices )
{
   return b->createIndexBuffer( transferList, count, pIndices );
}

BufferHandle CreateUniformBuffer( size_t size ) { return b->createUniformBuffer( size ); }

BufferHandle CreateBuffer( size_t size ) { return b->createBuffer( size ); }

void CopyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size )
{
   return b->copyToBuffer( bufferHandle, pData, offset, size );
}

void DestroyTexture( TextureHandle texHandle ) { b->destroyTexture( texHandle ); }

void DestroyVertexBuffer( VertexBufferHandle bufferHandle )
{
   b->destroyVertexBuffer( bufferHandle );
}

void DestroyIndexBuffer( IndexBufferHandle bufferHandle ) { b->destroyIndexBuffer( bufferHandle ); }

void DestroyBuffer( BufferHandle bufferHandle ) { b->destroyBuffer( bufferHandle ); }

// =================================================================================================
// Drawing
//
void PrepareFrame() { b->prepareFrame(); }

void BeginRendering( CmdListHandle cmdList, bool wantDepth )
{
   b->beginRendering( cmdList, wantDepth );
}

void BeginRendering(
    CmdListHandle cmdList,
    const RenderTargetsInfo& targetsInfo,
    const std::vector<TextureHandle>& targets )
{
   b->beginRendering( cmdList, targetsInfo, targets );
}

void NextPass( CmdListHandle cmdList ) { b->nextPass( cmdList ); }

void EndRendering( CmdListHandle cmdList ) { b->endRendering( cmdList ); }

void DrawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex )
{
   b->drawVertices( cmdList, vertexCount, firstVertex );
}

void DrawVerticesIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex )
{
   b->drawVerticesIndexed( cmdList, indexCount, firstIndex );
}

void Dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ )
{
   b->dispatch( cmdList, workX, workY, workZ );
}

void PresentFrame() { b->presentFrame(); }
}
