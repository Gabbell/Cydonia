#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <Handles/Handle.h>

namespace cyd
{
class RenderBackend
{
  public:
   RenderBackend() = default;
   NON_COPIABLE( RenderBackend );
   virtual ~RenderBackend() = default;

   virtual void cleanup() = 0;

   // ==============================================================================================
   // Command Buffers/Lists

   virtual CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable ) = 0;

   virtual void startRecordingCommandList( CmdListHandle cmdList ) = 0;
   virtual void endRecordingCommandList( CmdListHandle cmdList )   = 0;
   virtual void submitCommandList( CmdListHandle cmdList )         = 0;

   virtual void waitOnCommandList( CmdListHandle cmdList )  = 0;
   virtual void destroyCommandList( CmdListHandle cmdList ) = 0;

   // ==============================================================================================
   // Pipeline Specification

   virtual void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )           = 0;
   virtual void bindTexture( CmdListHandle cmdList, TextureHandle texHandle )                = 0;
   virtual void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle ) = 0;
   virtual void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )   = 0;
   virtual void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )     = 0;
   virtual void setViewport( CmdListHandle cmdList, const Rectangle& viewport )              = 0;

   // ==============================================================================================
   // Resources

   virtual TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const ShaderObjectInfo& info,
       const DescriptorSetLayoutInfo& layout,
       const void* texels ) = 0;

   virtual VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* vertices ) = 0;

   virtual IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* indices ) = 0;

   virtual UniformBufferHandle createUniformBuffer(
       size_t size,
       const ShaderObjectInfo& info,
       const DescriptorSetLayoutInfo& layout ) = 0;

   virtual void
   mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* data ) = 0;

   virtual void destroyTexture( TextureHandle texHandle )                = 0;
   virtual void destroyVertexBuffer( VertexBufferHandle bufferHandle )   = 0;
   virtual void destroyIndexBuffer( IndexBufferHandle bufferHandle )     = 0;
   virtual void destroyUniformBuffer( UniformBufferHandle bufferHandle ) = 0;

   // ==============================================================================================
   // Drawing

   virtual void beginRenderPass( CmdListHandle cmdList )                 = 0;
   virtual void endRenderPass( CmdListHandle cmdList )                   = 0;
   virtual void drawFrame( CmdListHandle cmdList, uint32_t vertexCount ) = 0;
   // void drawFrameIndexed( CmdListHandle handle, uint32_t indexCount );
   virtual void presentFrame() = 0;
};
}