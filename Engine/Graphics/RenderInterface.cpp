#include <Graphics/RenderInterface.h>

#include <Graphics/Backends/VKRenderBackend.h>

#include <cstdio>

namespace cyd::GRIS
{
static RenderBackend* b = nullptr;

// =================================================================================================
// Initialization

template <>
void InitRenderBackend<VK>( const Window& window )
{
   printf( "======= Initializing Vulkan Rendering Backend =======\n" );
   delete b;
   b = new VKRenderBackend( window );
}

template <>
void InitRenderBackend<GL>( const Window& )
{
   printf( "======= OpenGL Rendering Backend Not Yet Implemented =======\n" );
   delete b;
}

void UninitRenderBackend()
{
   printf( "======= Uninitializing Rendering Backend =======\n" );
   delete b;
}

void RenderBackendCleanup() { b->cleanup(); }

// =================================================================================================
// Command Buffers/Lists

CmdListHandle CreateCommandList( QueueUsageFlag usage, bool presentable )
{
   return b->createCommandList( usage, presentable );
}

void SubmitCommandList( CmdListHandle cmdList ) { b->submitCommandList( cmdList ); }
void StartRecordingCommandList( CmdListHandle cmdList ) { b->startRecordingCommandList( cmdList ); }
void EndRecordingCommandList( CmdListHandle cmdList ) { b->endRecordingCommandList( cmdList ); }

void WaitOnCommandList( CmdListHandle cmdList ) { b->waitOnCommandList( cmdList ); }
void DestroyCommandList( CmdListHandle cmdList ) { b->destroyCommandList( cmdList ); }

// =================================================================================================
// Pipeline Specification

void BindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void BindTexture( CmdListHandle cmdList, TextureHandle texHandle )
{
   b->bindTexture( cmdList, texHandle );
}

void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   b->bindVertexBuffer( cmdList, bufferHandle );
}

void BindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
{
   b->bindIndexBuffer( cmdList, bufferHandle );
}

void BindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle )
{
   b->bindUniformBuffer( cmdList, bufferHandle );
}

void SetViewport( CmdListHandle cmdList, const Rectangle& viewport )
{
   b->setViewport( cmdList, viewport );
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

TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    uint32_t shaderObjectIdx,
    const DescriptorSetLayoutInfo& layout,
    const void* pTexels )
{
   return b->createTexture( transferList, desc, shaderObjectIdx, layout, pTexels );
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

UniformBufferHandle
CreateUniformBuffer( size_t size, uint32_t shaderObjectIdx, const DescriptorSetLayoutInfo& layout )
{
   return b->createUniformBuffer( size, shaderObjectIdx, layout );
}

void MapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* pData )
{
   return b->mapUniformBufferMemory( bufferHandle, pData );
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

void BeginRenderPass( CmdListHandle cmdList ) { b->beginRenderPass( cmdList ); }

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
