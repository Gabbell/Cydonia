#pragma once

#include <Graphics/GraphicsTypes.h>
#include <Graphics/RenderPipelines.h>
#include <Graphics/Handles/ResourceHandle.h>

// =================================================================================================
// Graphics Rendering Interface Subsystem
// =================================================================================================
namespace CYD
{
class Window;
struct GraphicsPipelineInfo;
struct ComputePipelineInfo;

namespace GRIS
{
enum API
{
   VK,
   D3D12,
   D3D11,
   GL,
   MTL
};

// Initialization
bool InitRenderBackend( API api, const Window& window );
void UninitRenderBackend();
void RenderBackendCleanup();  // Should be called every frame

// Command Buffers/Lists
CmdListHandle CreateCommandList( QueueUsageFlag usage, bool presentable = false );
void StartRecordingCommandList( CmdListHandle cmdList );
void EndRecordingCommandList( CmdListHandle cmdList );
void SubmitCommandList( CmdListHandle cmdList );
void ResetCommandList( CmdListHandle cmdList );
void WaitOnCommandList( CmdListHandle cmdList );
void DestroyCommandList( CmdListHandle cmdList );

// Dynamic state
// TODO Dynamic depth? Some APIs may have different min/max depth conventions
void SetViewport( CmdListHandle cmdList, const Viewport& viewport );
void SetScissor( CmdListHandle cmdList, const Rectangle& scissor );

// Bind shader pipelines
void BindPipeline( CmdListHandle cmdList, const std::string_view pipName );
void BindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo );
void BindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo );

// Bind vertex and index buffers
void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );
template <class T>
void BindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle );

// Bind shader resources by binding/set
void BindTexture( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding );
void BindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding );
void BindBuffer( CmdListHandle cmdList, BufferHandle bufferHandle, uint32_t set, uint32_t binding );
void BindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t set,
    uint32_t binding );

// Bind shader resources by name. Using .json definitions
void BindTexture( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name );
void BindImage( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name );
void BindBuffer( CmdListHandle cmdList, BufferHandle bufferHandle, const std::string_view name );
void BindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    const std::string_view name );

void UpdateConstantBuffer(
    CmdListHandle cmdList,
    ShaderStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData );

// Resources
TextureHandle CreateTexture( CmdListHandle transferList, const TextureDescription& desc );
TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    const std::string& path );
TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    const std::vector<std::string>& paths );
TextureHandle
CreateTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels );
VertexBufferHandle CreateVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices );
IndexBufferHandle
CreateIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices );
BufferHandle CreateUniformBuffer( size_t size );
BufferHandle CreateBuffer( size_t size );
void CopyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size );

void DestroyTexture( TextureHandle texHandle );
void DestroyVertexBuffer( VertexBufferHandle bufferHandle );
void DestroyIndexBuffer( IndexBufferHandle bufferHandle );
void DestroyBuffer( BufferHandle bufferHandle );

// Drawing
void PrepareFrame();
void BeginRendering( CmdListHandle cmdList, bool wantDepth = false );
void BeginRendering(
    CmdListHandle cmdList,
    const RenderTargetsInfo& attachmentsInfo,
    const std::vector<TextureHandle>& targets );
void NextPass( CmdListHandle cmdList );
void EndRendering( CmdListHandle cmdList );
void DrawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex = 0 );
void DrawVerticesIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex = 0 );
void Dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ );
void PresentFrame();
}
}
