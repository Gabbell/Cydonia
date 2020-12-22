#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
struct GraphicsPipelineInfo;
struct ComputePipelineInfo;

class RenderBackend
{
  public:
   RenderBackend() = default;
   NON_COPIABLE( RenderBackend );
   virtual ~RenderBackend() = default;

   virtual void cleanup() = 0;

   // Command Buffers/Lists
   // ==============================================================================================
   virtual CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable ) = 0;

   virtual void startRecordingCommandList( CmdListHandle cmdList ) = 0;
   virtual void endRecordingCommandList( CmdListHandle cmdList )   = 0;
   virtual void submitCommandList( CmdListHandle cmdList )         = 0;
   virtual void resetCommandList( CmdListHandle cmdList )          = 0;
   virtual void waitOnCommandList( CmdListHandle cmdList )         = 0;
   virtual void destroyCommandList( CmdListHandle cmdList )        = 0;

   // Pipeline Specification
   // ==============================================================================================
   virtual void setViewport( CmdListHandle cmdList, const Viewport& viewport )             = 0;
   virtual void setScissor( CmdListHandle cmdList, const Rectangle& scissor )              = 0;
   virtual void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ) = 0;
   virtual void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo )  = 0;
   virtual void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) = 0;
   virtual void
   bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle, IndexType type ) = 0;
   virtual void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       uint32_t set,
       uint32_t binding ) = 0;
   virtual void
   bindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding ) = 0;
   virtual void bindBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding ) = 0;
   virtual void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding ) = 0;
   virtual void
   bindTexture( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name ) = 0;
   virtual void
   bindImage( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name ) = 0;
   virtual void
   bindBuffer( CmdListHandle cmdList, BufferHandle bufferHandle, const std::string_view name ) = 0;
   virtual void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       const std::string_view name ) = 0;
   virtual void updateConstantBuffer(
       CmdListHandle cmdList,
       ShaderStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData ) = 0;

   // Resources
   // ==============================================================================================
   virtual TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc ) = 0;
   virtual TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const std::string& path ) = 0;
   virtual TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const std::vector<std::string>& paths ) = 0;
   virtual TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const void* pTexels ) = 0;

   virtual VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices ) = 0;

   virtual IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices ) = 0;

   virtual BufferHandle createUniformBuffer( size_t size ) = 0;

   virtual BufferHandle createBuffer( size_t size ) = 0;

   virtual void
   copyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size ) = 0;

   virtual void destroyTexture( TextureHandle texHandle )              = 0;
   virtual void destroyVertexBuffer( VertexBufferHandle bufferHandle ) = 0;
   virtual void destroyIndexBuffer( IndexBufferHandle bufferHandle )   = 0;
   virtual void destroyBuffer( BufferHandle bufferHandle )             = 0;

   // Drawing
   // ==============================================================================================
   virtual void prepareFrame()                                          = 0;
   virtual void beginRendering( CmdListHandle cmdList, bool wantDepth ) = 0;
   virtual void beginRendering(
       CmdListHandle cmdList,
       const RenderTargetsInfo& targetsInfo,
       const std::vector<TextureHandle>& targets )                                            = 0;
   virtual void nextPass( CmdListHandle cmdList )                                             = 0;
   virtual void endRendering( CmdListHandle cmdList )                                         = 0;
   virtual void drawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) = 0;
   virtual void
   drawVerticesIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ) = 0;
   virtual void
   dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ ) = 0;
   virtual void presentFrame()                                                       = 0;
};
}
