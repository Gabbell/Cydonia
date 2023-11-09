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
class Framebuffer;
class VertexList;
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
bool InitializeUIBackend();
void UninitializeUIBackend();
void DrawUI( CmdListHandle cmdList );

// Cleanup rendering resources
void RenderBackendCleanup();
void ReloadShaders();

// Wait until all devices are idle
void WaitUntilIdle();

// Command Buffers/Lists
// ===============================================================================================
CmdListHandle CreateCommandList(
    QueueUsageFlag usage,
    const std::string_view name = "",
    bool presentable            = false );
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
void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle );

template <class Type>
void BindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle, uint32_t offset = 0 );

// Bind shader resources by binding/set
void BindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t binding,
    uint32_t set = 0 );
void BindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const SamplerInfo& sampler,
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

VertexBufferHandle CreateVertexBuffer( size_t size, const std::string_view name );
IndexBufferHandle CreateIndexBuffer( size_t size, const std::string_view name );

BufferHandle CreateUniformBuffer( size_t size, const std::string_view name );
BufferHandle CreateBuffer( size_t size, const std::string_view name );

void* AddDebugTexture( TextureHandle texture );
void UpdateDebugTexture( CmdListHandle cmdList, TextureHandle texture );
void RemoveDebugTexture( void* texture );

void UploadToBuffer( BufferHandle bufferHandle, const void* pData, const UploadToBufferInfo& info );
void UploadToVertexBuffer(
    CmdListHandle transferList,
    VertexBufferHandle bufferHandle,
    const VertexList& vertices );
void UploadToIndexBuffer(
    CmdListHandle transferList,
    IndexBufferHandle bufferHandle,
    const void* pIndices,
    const UploadToBufferInfo& info );

void CopyTexture(
    CmdListHandle transferList,
    TextureHandle srcTexHandle,
    TextureHandle dstTexHandle,
    const TextureCopyInfo& info );

void DestroyTexture( TextureHandle texHandle );
void DestroyVertexBuffer( VertexBufferHandle bufferHandle );
void DestroyIndexBuffer( IndexBufferHandle bufferHandle );
void DestroyBuffer( BufferHandle bufferHandle );

// Drawing and Presentation
// ===============================================================================================
void BeginFrame();
void BeginRendering( CmdListHandle cmdList );
void BeginRendering( CmdListHandle cmdList, const Framebuffer& fb );
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
void ClearTexture( CmdListHandle cmdList, TextureHandle texHandle, const ClearValue& clearVal );
void CopyToSwapchain( CmdListHandle cmdList, TextureHandle texHandle );
void PresentFrame();

// Debug
// ===============================================================================================
void BeginDebugRange( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color );
void EndDebugRange( CmdListHandle cmdList );
void InsertDebugLabel( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color );
}
}
