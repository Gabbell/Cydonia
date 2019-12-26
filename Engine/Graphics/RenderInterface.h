#pragma once

#include <Graphics/GraphicsTypes.h>

#include <Graphics/Handles/Handle.h>

// GPU Rendering Interface
// =================================================================================================
namespace cyd
{
class Window;

enum API
{
   VK,
   D3D12,
   GL
};

// Initialization
// =================================================================================================
template <API>
void initRenderBackend( const Window& window );
void uninitRenderBackend();
void renderBackendCleanup();  // Should be called every frame

// Command Buffers/Lists
// =================================================================================================
CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable = false );

void startRecordingCommandList( CmdListHandle cmdList );
void endRecordingCommandList( CmdListHandle cmdList );
void submitCommandList( CmdListHandle cmdList );

void waitOnCommandList( CmdListHandle cmdList );
void destroyCommandList( CmdListHandle cmdList );

// Pipeline Specification
// =================================================================================================

void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo );
void bindTexture( CmdListHandle cmdList, TextureHandle texHandle );
void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );
void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle );
void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle );
void setViewport( CmdListHandle cmdList, const Rectangle& viewport );

void updateConstantBuffer(
    CmdListHandle cmdList,
    ShaderStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData );

// Resources
// =================================================================================================
TextureHandle createTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    uint32_t shaderObjectIdx,
    const DescriptorSetLayoutInfo& layout,
    const void* pTexels );

VertexBufferHandle createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices );

IndexBufferHandle
createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices );

UniformBufferHandle
createUniformBuffer( size_t size, uint32_t shaderObjectIdx, const DescriptorSetLayoutInfo& layout );
void mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* pData );

void destroyTexture( TextureHandle texHandle );
void destroyVertexBuffer( VertexBufferHandle bufferHandle );
void destroyIndexBuffer( IndexBufferHandle bufferHandle );
void destroyUniformBuffer( UniformBufferHandle bufferHandle );

// Drawing
// =================================================================================================
void beginRenderPass( CmdListHandle cmdList );
void endRenderPass( CmdListHandle cmdList );
void drawVertices( CmdListHandle cmdList, uint32_t vertexCount );
void drawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount );
void presentFrame();
}
