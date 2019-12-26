#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <Graphics/Handles/Handle.h>

namespace cyd
{
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

   virtual void waitOnCommandList( CmdListHandle cmdList )  = 0;
   virtual void destroyCommandList( CmdListHandle cmdList ) = 0;

   // Pipeline Specification
   // ==============================================================================================
   virtual void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )           = 0;
   virtual void bindTexture( CmdListHandle cmdList, TextureHandle texHandle )                = 0;
   virtual void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle ) = 0;
   virtual void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )   = 0;
   virtual void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )     = 0;
   virtual void setViewport( CmdListHandle cmdList, const Rectangle& viewport )              = 0;

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
       const TextureDescription& desc,
       uint32_t shaderObjectIdx,
       const DescriptorSetLayoutInfo& layout,
       const void* pTexels ) = 0;

   virtual VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices ) = 0;

   virtual IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices ) = 0;

   virtual UniformBufferHandle createUniformBuffer(
       size_t size,
       uint32_t shaderObjectIdx,
       const DescriptorSetLayoutInfo& layout )                                               = 0;
   virtual void mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* pData ) = 0;

   virtual void destroyTexture( TextureHandle texHandle )                = 0;
   virtual void destroyVertexBuffer( VertexBufferHandle bufferHandle )   = 0;
   virtual void destroyIndexBuffer( IndexBufferHandle bufferHandle )     = 0;
   virtual void destroyUniformBuffer( UniformBufferHandle bufferHandle ) = 0;

   // Drawing
   // ==============================================================================================
   virtual void beginRenderPass( CmdListHandle cmdList )                          = 0;
   virtual void endRenderPass( CmdListHandle cmdList )                            = 0;
   virtual void drawVertices( CmdListHandle cmdList, uint32_t vertexCount )       = 0;
   virtual void drawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount ) = 0;
   virtual void presentFrame()                                                    = 0;
};
}