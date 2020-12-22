#pragma once

#include <Graphics/GRIS/Backends/RenderBackend.h>

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

// =================================================================================================
// Forwards
// =================================================================================================
namespace CYD
{
class Window;
class VKRenderBackendImp;
}

// =================================================================================================
// Definition
// =================================================================================================
namespace CYD
{
class VKRenderBackend final : public RenderBackend
{
  public:
   VKRenderBackend( const Window& window );
   NON_COPIABLE( VKRenderBackend );
   virtual ~VKRenderBackend();

   void cleanup() override;

   // Command Buffers/Lists
   // ==============================================================================================
   CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable ) override;

   void startRecordingCommandList( CmdListHandle cmdList ) override;
   void endRecordingCommandList( CmdListHandle cmdList ) override;
   void submitCommandList( CmdListHandle cmdList ) override;
   void resetCommandList( CmdListHandle cmdList ) override;
   void waitOnCommandList( CmdListHandle cmdList ) override;
   void destroyCommandList( CmdListHandle cmdList ) override;

   // Pipeline Specification
   // ==============================================================================================
   void setViewport( CmdListHandle cmdList, const Viewport& viewport ) override;
   void setScissor( CmdListHandle cmdList, const Rectangle& scissor ) override;

   void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ) override;
   void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo ) override;

   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) override;
   void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle, IndexType type )
       override;

   void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       uint32_t set,
       uint32_t binding ) override;
   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding )
       override;
   void bindBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding ) override;
   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding ) override;

   void bindTexture( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name )
       override;
   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name )
       override;
   void bindBuffer( CmdListHandle cmdList, BufferHandle bufferHandle, const std::string_view name )
       override;
   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       const std::string_view name ) override;

   void updateConstantBuffer(
       CmdListHandle cmdList,
       ShaderStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData ) override;

   // Resources
   // ==============================================================================================
   TextureHandle createTexture( CmdListHandle transferList, const TextureDescription& desc )
       override;

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const std::string& path ) override;

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const std::vector<std::string>& paths ) override;

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const void* pTexels ) override;

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices ) override;

   IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices ) override;

   BufferHandle createUniformBuffer( size_t size ) override;

   BufferHandle createBuffer( size_t size ) override;

   void copyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size )
       override;

   void destroyTexture( TextureHandle texHandle ) override;
   void destroyVertexBuffer( VertexBufferHandle bufferHandle ) override;
   void destroyIndexBuffer( IndexBufferHandle bufferHandle ) override;
   void destroyBuffer( BufferHandle bufferHandle ) override;

   // Drawing
   // ==============================================================================================
   void prepareFrame() override;
   void beginRendering( CmdListHandle cmdList, bool wantDepth ) override;
   void beginRendering(
       CmdListHandle cmdList,
       const RenderTargetsInfo& targetsInfo,
       const std::vector<TextureHandle>& targets ) override;
   void nextPass( CmdListHandle cmdList ) override;
   void endRendering( CmdListHandle cmdList ) override;
   void drawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) override;
   void drawVerticesIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ) override;
   void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ );
   void presentFrame() override;

  private:
   VKRenderBackendImp* _imp;
};
}
