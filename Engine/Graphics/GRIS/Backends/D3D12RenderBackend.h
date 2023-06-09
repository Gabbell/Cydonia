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
class D3D12RenderBackendImp;
}

// =================================================================================================
// Definition
// =================================================================================================
namespace CYD
{
class D3D12RenderBackend final : public RenderBackend
{
  public:
   D3D12RenderBackend( const Window& window );
   NON_COPIABLE( D3D12RenderBackend );
   virtual ~D3D12RenderBackend();

   void cleanup() override;

   void waitUntilIdle() override;

   // Command Buffers/Lists
   // ==============================================================================================
   CmdListHandle getMainCommandList() const override;
   CmdListHandle createCommandList(
       QueueUsageFlag usage,
       const std::string_view name,
       bool presentable ) override;

   void submitCommandList( CmdListHandle cmdList ) override;
   void resetCommandList( CmdListHandle cmdList ) override;
   void waitOnCommandList( CmdListHandle cmdList ) override;
   void syncOnCommandList( CmdListHandle from, CmdListHandle to ) override;
   void destroyCommandList( CmdListHandle cmdList ) override;

   void syncOnSwapchain( CmdListHandle cmdList ) override;
   void syncToSwapchain( CmdListHandle cmdList ) override;

   // Pipeline Specification
   // ==============================================================================================
   void setViewport( CmdListHandle cmdList, const Viewport& viewport ) override;
   void setScissor( CmdListHandle cmdList, const Rectangle& scissor ) override;

   void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ) override;
   void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo ) override;

   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) override;
   void bindIndexBuffer(
       CmdListHandle cmdList,
       IndexBufferHandle bufferHandle,
       IndexType type,
       uint32_t offset ) override;

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
       uint32_t binding,
       uint32_t set,
       uint32_t offset,
       uint32_t range ) override;
   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t binding,
       uint32_t set,
       uint32_t offset,
       uint32_t range ) override;

   void updateConstantBuffer(
       CmdListHandle cmdList,
       PipelineStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData ) override;

   // Resources
   // ==============================================================================================
   TextureHandle createTexture( const TextureDescription& desc ) override;

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const void* pTexels ) override;

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       uint32_t layerCount,
       const void* const* ppTexels ) override;

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices,
       const std::string_view name ) override;

   IndexBufferHandle createIndexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       const void* pIndices,
       const std::string_view name ) override;

   BufferHandle createUniformBuffer( size_t size, const std::string_view name ) override;

   BufferHandle createBuffer( size_t size, const std::string_view name ) override;

   void copyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size )
       override;

   void destroyTexture( TextureHandle texHandle ) override;
   void destroyVertexBuffer( VertexBufferHandle bufferHandle ) override;
   void destroyIndexBuffer( IndexBufferHandle bufferHandle ) override;
   void destroyBuffer( BufferHandle bufferHandle ) override;

   // Drawing
   // ==============================================================================================
   void prepareFrame() override;
   void beginRendering( CmdListHandle cmdList ) override;
   void beginRendering(
       CmdListHandle cmdList,
       const FramebufferInfo& targetsInfo,
       const std::vector<TextureHandle>& targets ) override;
   void nextPass( CmdListHandle cmdList ) override;
   void endRendering( CmdListHandle cmdList ) override;
   void drawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) override;
   void drawVerticesIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ) override;
   void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ );
   void presentFrame() override;

  private:
   D3D12RenderBackendImp* _imp;
};
}
