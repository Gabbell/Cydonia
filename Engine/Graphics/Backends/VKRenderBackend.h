#pragma once

#include <Graphics/Backends/RenderBackend.h>

#include <Common/Include.h>

#include <Graphics/Handles/Handle.h>

// =================================================================================================
// Forwards
// =================================================================================================
namespace cyd
{
class Window;
class VKRenderBackendImp;
}

// =================================================================================================
// Definition
// =================================================================================================
namespace cyd
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

   void submitCommandList( CmdListHandle cmdList ) override;
   void startRecordingCommandList( CmdListHandle cmdList ) override;
   void endRecordingCommandList( CmdListHandle cmdList ) override;

   void waitOnCommandList( CmdListHandle cmdList ) override;
   void destroyCommandList( CmdListHandle cmdList ) override;

   // Pipeline Specification
   // ==============================================================================================
   void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo ) override;
   void bindTexture( CmdListHandle cmdList, TextureHandle texHandle ) override;
   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) override;
   void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle ) override;
   void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle ) override;
   void setViewport( CmdListHandle cmdList, const Rectangle& viewport ) override;

   void updateConstantBuffer(
       CmdListHandle cmdList,
       ShaderStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData ) override;

   // Resources
   // ==============================================================================================
   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       uint32_t shaderObjectIdx,
       const DescriptorSetLayoutInfo& layout,
       const void* pTexels ) override;

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices ) override;

   IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices ) override;

   UniformBufferHandle createUniformBuffer(
       size_t size,
       uint32_t shaderObjectIdx,
       const DescriptorSetLayoutInfo& layout ) override;
   void mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* pData ) override;

   void destroyTexture( TextureHandle texHandle ) override;
   void destroyVertexBuffer( VertexBufferHandle bufferHandle ) override;
   void destroyIndexBuffer( IndexBufferHandle bufferHandle ) override;
   void destroyUniformBuffer( UniformBufferHandle bufferHandle ) override;

   // Drawing
   // ==============================================================================================
   void beginRenderPass( CmdListHandle cmdList ) override;
   void endRenderPass( CmdListHandle cmdList ) override;
   void drawVertices( CmdListHandle cmdList, uint32_t vertexCount ) override;
   void drawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount ) override;
   void presentFrame() override;

  private:
   VKRenderBackendImp* _imp;
};
}