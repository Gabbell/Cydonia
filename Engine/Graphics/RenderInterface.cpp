#include <Graphics/RenderInterface.h>

#include <Graphics/Backends/VKRenderBackend.h>

#include <cstdio>

namespace cyd::GRIS
{
static RenderBackend* b = nullptr;

// =================================================================================================
// Initialization
//
template <>
bool InitRenderBackend<VK>( const Window& window )
{
   printf( "======= Initializing Vulkan Rendering Backend =======\n" );
   delete b;
   b = new VKRenderBackend( window );
   return true;
}

template <>
bool InitRenderBackend<GL>( const Window& )
{
   printf( "======= OpenGL Rendering Backend Not Yet Implemented =======\n" );
   delete b;
   return false;
}

void UninitRenderBackend()
{
   printf( "======= Uninitializing Rendering Backend =======\n" );
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
void SetViewport( CmdListHandle cmdList, const Rectangle& viewport )
{
   b->setViewport( cmdList, viewport );
}

void BindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   b->bindVertexBuffer( cmdList, bufferHandle );
}

void BindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
{
   b->bindIndexBuffer( cmdList, bufferHandle );
}

void BindTexture( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding )
{
   b->bindTexture( cmdList, texHandle, set, binding );
}

void BindUniformBuffer(
    CmdListHandle cmdList,
    UniformBufferHandle bufferHandle,
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

UniformBufferHandle CreateUniformBuffer( size_t size ) { return b->createUniformBuffer( size ); }

void CopyToUniformBuffer( UniformBufferHandle bufferHandle, const void* pData )
{
   return b->copyToUniformBuffer( bufferHandle, pData );
}

void DestroyTexture( TextureHandle texHandle ) { b->destroyTexture( texHandle ); }

void DestroyVertexBuffer( VertexBufferHandle bufferHandle )
{
   b->destroyVertexBuffer( bufferHandle );
}

void DestroyIndexBuffer( IndexBufferHandle bufferHandle ) { b->destroyIndexBuffer( bufferHandle ); }

void DestroyUniformBuffer( UniformBufferHandle bufferHandle )
{
   b->destroyUniformBuffer( bufferHandle );
}

// =================================================================================================
// Drawing
//
void BeginRenderPassSwapchain( CmdListHandle cmdList, const RenderPassInfo& renderPassInfo )
{
   b->beginRenderSwapchain( cmdList, renderPassInfo );
}

void EndRenderPass( CmdListHandle cmdList ) { b->endRenderPass( cmdList ); }

void DrawVertices( CmdListHandle cmdList, uint32_t vertexCount )
{
   b->drawVertices( cmdList, vertexCount );
}

void DrawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount )
{
   b->drawVerticesIndexed( cmdList, indexCount );
}

void PresentFrame() { b->presentFrame(); }
}
