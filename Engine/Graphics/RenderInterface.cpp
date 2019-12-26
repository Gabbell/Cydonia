#include <Graphics/RenderInterface.h>

#include <Graphics/Backends/VKRenderBackend.h>

#include <cstdio>

namespace cyd
{
static RenderBackend* b = nullptr;

// =================================================================================================
// Initialization

template <>
void initRenderBackend<API::VK>( const Window& window )
{
   printf( "======= Initializing Vulkan Rendering Backend =======\n" );
   delete b;
   b = new VKRenderBackend( window );
}

template <>
void initRenderBackend<API::GL>( const Window& )
{
   printf( "======= Initializing OpenGL Rendering Backend =======\n" );
   delete b;
}

void uninitRenderBackend()
{
   printf( "======= Uninitializing Rendering Backend =======\n" );
   delete b;
}

void renderBackendCleanup() { b->cleanup(); }

// =================================================================================================
// Command Buffers/Lists

CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable )
{
   return b->createCommandList( usage, presentable );
}

void submitCommandList( CmdListHandle cmdList ) { b->submitCommandList( cmdList ); }
void startRecordingCommandList( CmdListHandle cmdList ) { b->startRecordingCommandList( cmdList ); }
void endRecordingCommandList( CmdListHandle cmdList ) { b->endRecordingCommandList( cmdList ); }

void waitOnCommandList( CmdListHandle cmdList ) { b->waitOnCommandList( cmdList ); }
void destroyCommandList( CmdListHandle cmdList ) { b->destroyCommandList( cmdList ); }

// =================================================================================================
// Pipeline Specification

void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void bindTexture( CmdListHandle cmdList, TextureHandle texHandle )
{
   b->bindTexture( cmdList, texHandle );
}

void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   b->bindVertexBuffer( cmdList, bufferHandle );
}

void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
{
   b->bindIndexBuffer( cmdList, bufferHandle );
}

void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle )
{
   b->bindUniformBuffer( cmdList, bufferHandle );
}

void setViewport( CmdListHandle cmdList, const Rectangle& viewport )
{
   b->setViewport( cmdList, viewport );
}

void updateConstantBuffer(
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

TextureHandle createTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    uint32_t shaderObjectIdx,
    const DescriptorSetLayoutInfo& layout,
    const void* pTexels )
{
   return b->createTexture( transferList, desc, shaderObjectIdx, layout, pTexels );
}

VertexBufferHandle createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices )
{
   return b->createVertexBuffer( transferList, count, stride, pVertices );
}

IndexBufferHandle
createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices )
{
   return b->createIndexBuffer( transferList, count, pIndices );
}

UniformBufferHandle
createUniformBuffer( size_t size, uint32_t shaderObjectIdx, const DescriptorSetLayoutInfo& layout )
{
   return b->createUniformBuffer( size, shaderObjectIdx, layout );
}

void mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* pData )
{
   return b->mapUniformBufferMemory( bufferHandle, pData );
}

void destroyTexture( TextureHandle texHandle ) { b->destroyTexture( texHandle ); }

void destroyVertexBuffer( VertexBufferHandle bufferHandle )
{
   b->destroyVertexBuffer( bufferHandle );
}

void destroyIndexBuffer( IndexBufferHandle bufferHandle ) { b->destroyIndexBuffer( bufferHandle ); }

void destroyUniformBuffer( UniformBufferHandle bufferHandle )
{
   b->destroyUniformBuffer( bufferHandle );
}

// =================================================================================================
// Drawing

void beginRenderPass( CmdListHandle cmdList ) { b->beginRenderPass( cmdList ); }

void endRenderPass( CmdListHandle cmdList ) { b->endRenderPass( cmdList ); }

void drawVertices( CmdListHandle cmdList, uint32_t vertexCount )
{
   b->drawVertices( cmdList, vertexCount );
}

void drawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount )
{
   b->drawVerticesIndexed( cmdList, indexCount );
}

void presentFrame() { b->presentFrame(); }
}
