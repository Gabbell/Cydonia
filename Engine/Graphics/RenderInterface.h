#pragma once

#include <Graphics/GraphicsTypes.h>

#include <Handles/Handle.h>

namespace cyd
{
// =================================================================================================
// Forwards
class Window;

// =================================================================================================
// Render APIs
enum API
{
   VK,
   D3D12,
   GL
};

// =================================================================================================
// Initialization

template <API>
void initRenderBackend( const Window& window );
void uninitRenderBackend();
void renderBackendCleanup();  // Should be called every frame

// =================================================================================================
// Command Buffers/Lists

CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable = false );

void startRecordingCommandList( CmdListHandle cmdList );
void endRecordingCommandList( CmdListHandle cmdList );
void submitCommandList( CmdListHandle cmdList );

void waitOnCommandList( CmdListHandle cmdList );
void destroyCommandList( CmdListHandle cmdList );

// =================================================================================================
// Pipeline Specification

void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo );
void bindTexture( CmdListHandle cmdList, TextureHandle texHandle );
void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );
void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle );
void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle );
void setViewport( CmdListHandle cmdList, const Rectangle& viewport );

// =================================================================================================
// Resources

TextureHandle createTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    const ShaderObjectInfo& info,
    const DescriptorSetLayoutInfo& layout,
    const void* texels );

VertexBufferHandle createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* vertices );

IndexBufferHandle
createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* indices );

UniformBufferHandle createUniformBuffer(
    size_t size,
    const ShaderObjectInfo& info,
    const DescriptorSetLayoutInfo& layout );
void mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* data );

void destroyTexture( TextureHandle texHandle );
void destroyVertexBuffer( VertexBufferHandle bufferHandle );
void destroyIndexBuffer( IndexBufferHandle bufferHandle );
void destroyUniformBuffer( UniformBufferHandle bufferHandle );

// =================================================================================================
// Drawing

void beginRenderPass( CmdListHandle cmdList );
void endRenderPass( CmdListHandle cmdList );
void drawFrame( CmdListHandle cmdList, uint32_t vertexCount );
// void drawFrameIndexed( CmdListHandle handle, uint32_t indexCount );
void presentFrame();
}
