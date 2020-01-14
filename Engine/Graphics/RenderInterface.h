#pragma once

#include <Graphics/GraphicsTypes.h>

#include <Handles/Handle.h>

// =================================================================================================
// Graphics Rendering Interface Subsystem
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

namespace GRIS
{
// Initialization
template <API>
void InitRenderBackend( const Window& window );
void UninitRenderBackend();
void RenderBackendCleanup();  // Should be called every frame

// Command Buffers/Lists
CmdListHandle CreateCommandList( QueueUsageFlag usage, bool presentable = false );

void StartRecordingCommandList( CmdListHandle cmdList );
void EndRecordingCommandList( CmdListHandle cmdList );
void SubmitCommandList( CmdListHandle cmdList );

void WaitOnCommandList( CmdListHandle cmdList );
void DestroyCommandList( CmdListHandle cmdList );

// Pipeline Specification
void BindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo );
void BindTexture( CmdListHandle cmdList, TextureHandle texHandle );
void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );
void BindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle );
void BindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle );
void SetViewport( CmdListHandle cmdList, const Rectangle& viewport );
void UpdateConstantBuffer(
    CmdListHandle cmdList,
    ShaderStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData );

// Resources
TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    uint32_t shaderObjectIdx,
    const DescriptorSetLayoutInfo& layout,
    const void* pTexels );

VertexBufferHandle CreateVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices );

IndexBufferHandle
CreateIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices );

UniformBufferHandle
CreateUniformBuffer( size_t size, uint32_t shaderObjectIdx, const DescriptorSetLayoutInfo& layout );
void MapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* pData );

void DestroyTexture( TextureHandle texHandle );
void DestroyVertexBuffer( VertexBufferHandle bufferHandle );
void DestroyIndexBuffer( IndexBufferHandle bufferHandle );
void DestroyUniformBuffer( UniformBufferHandle bufferHandle );

// Drawing
void BeginRenderPass( CmdListHandle cmdList );
void EndRenderPass( CmdListHandle cmdList );
void DrawVertices( CmdListHandle cmdList, uint32_t vertexCount );
void DrawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount );
void PresentFrame();
}
}
