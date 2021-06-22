#pragma once

#include <Graphics/GraphicsTypes.h>
#include <Graphics/RenderPipelines.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <array>
#include <string>
#include <string_view>
#include <vector>

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
   NONE,
   VK,     // Vulkan
   D3D12,  // DirectX 12 (Not implemented)
   D3D11,  // DirectX 11 (Not implemented)
   GL,     // OpenGL (Not implemented)
   MTL     // Metal (Not implemented)
};

// Core initialization
bool InitRenderBackend( API api, const Window& window );
void UninitRenderBackend();

// For API specific implementations of UI
bool InitializeUI();
void UninitializeUI();
void DrawUI( CmdListHandle cmdList );

// Cleanup rendering resources
void RenderBackendCleanup();

// Command Buffers/Lists
// ===============================================================================================
CmdListHandle CreateCommandList(
    QueueUsageFlag usage,
    const std::string_view name = "",
    bool presentable            = false );
void StartRecordingCommandList( CmdListHandle cmdList );
void EndRecordingCommandList( CmdListHandle cmdList );
void SubmitCommandList( CmdListHandle cmdList );

// TODO Parallel work submission
// Will submit the command lists in as much of parallel way as possible (multiple queues). If
// these command lists have any inter-dependencies, they should be specified with GPU semaphores.
void SubmitCommandLists( std::vector<CmdListHandle> /*cmdLists*/ );

void ResetCommandList( CmdListHandle cmdList );
void WaitOnCommandList( CmdListHandle cmdList );  // Spinlock until command list finishes execution
void SyncOnCommandList( CmdListHandle from, CmdListHandle to );
void DestroyCommandList( CmdListHandle cmdList );

// Tying rendering synchronization together
void SyncOnSwapchain( CmdListHandle cmdList );
void SyncToSwapchain( CmdListHandle cmdList );

// Pipeline
// ===============================================================================================

// Dynamic state
// TODO Dynamic depth? Some APIs may have different min/max depth conventions
void SetViewport( CmdListHandle cmdList, const Viewport& viewport );
void SetScissor( CmdListHandle cmdList, const Rectangle& scissor );

void BindPipeline( CmdListHandle cmdList, const std::string_view pipName );
void BindPipeline( CmdListHandle cmdList, const PipelineInfo* pipInfo );
void BindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo );
void BindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo );

// Bind vertex and index buffers
template <class VertexLayout>
void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );

template <class Type>
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
// ===============================================================================================
TextureHandle CreateTexture( const TextureDescription& desc );
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
    const void* pVertices,
    const std::string_view name );

IndexBufferHandle CreateIndexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    const void* pIndices,
    const std::string_view name );

BufferHandle CreateUniformBuffer( size_t size, const std::string_view name );
BufferHandle CreateBuffer( size_t size, const std::string_view name );

void CopyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size );

void DestroyTexture( TextureHandle texHandle );
void DestroyVertexBuffer( VertexBufferHandle bufferHandle );
void DestroyIndexBuffer( IndexBufferHandle bufferHandle );
void DestroyBuffer( BufferHandle bufferHandle );

// Drawing and Presentation
// ===============================================================================================
void PrepareFrame();
void BeginRendering( CmdListHandle cmdList );
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

// Debug
// ===============================================================================================
void BeginDebugRange( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color );
void EndDebugRange( CmdListHandle cmdList );
void InsertDebugLabel(
    CmdListHandle cmdList,
    const char* name,
    const std::array<float, 4>& color );
}
}
