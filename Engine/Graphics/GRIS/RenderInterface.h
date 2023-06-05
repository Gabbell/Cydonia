#pragma once

#include <Graphics/GraphicsTypes.h>
#include <Graphics/PipelineInfos.h>
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
enum class API
{
   NONE,
   VK,     // Vulkan
   D3D11,  // DirectX 11 (Not implemented)
   D3D12,  // DirectX 12 (WIP)
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

// Wait until all devices are idle
void WaitUntilIdle();

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

void BindPipeline( CmdListHandle cmdList, PipelineIndex index );
void BindPipeline( CmdListHandle cmdList, const PipelineInfo* pipInfo );
void BindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo );
void BindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo );

// Bind vertex and index buffers
template <class VertexLayout>
void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );

template <class Type>
void BindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle, uint32_t offset = 0 );

// Bind shader resources by binding/set
void BindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t binding,
    uint32_t set = 0 );
void BindImage(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t binding,
    uint32_t set = 0 );
void BindBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set    = 0,
    uint32_t offset = 0,
    uint32_t range  = 0 );
void BindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set    = 0,
    uint32_t offset = 0,
    uint32_t range  = 0 );

void UpdateConstantBuffer(
    CmdListHandle cmdList,
    PipelineStageFlag stages,
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

void* AddDebugTexture( TextureHandle texture );
void RemoveDebugTexture( void* texture );

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
    const FramebufferInfo& attachmentsInfo,
    const std::vector<TextureHandle>& targets );
void NextPass( CmdListHandle cmdList );
void EndRendering( CmdListHandle cmdList );
void Draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex = 0 );
void DrawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex = 0 );
void DrawInstanced(
    CmdListHandle cmdList,
    size_t vertexCount,
    size_t instanceCount,
    size_t firstVertex   = 0,
    size_t firstInstance = 0 );
void DrawIndexedInstanced(
    CmdListHandle cmdList,
    size_t indexCount,
    size_t instanceCount,
    size_t firstIndex    = 0,
    size_t firstInstance = 0 );
void Dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ );
void PresentFrame();

// Debug
// ===============================================================================================
void BeginDebugRange( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color );
void EndDebugRange( CmdListHandle cmdList );
void InsertDebugLabel( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color );
}
}
