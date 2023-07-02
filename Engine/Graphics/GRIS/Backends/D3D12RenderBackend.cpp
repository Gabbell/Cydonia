#pragma once

#include <Graphics/GRIS/Backends/D3D12RenderBackend.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandleManager.h>

#include <Graphics/Direct3D12/Factory.h>
#include <Graphics/Direct3D12/Device.h>
#include <Graphics/Direct3D12/DeviceManager.h>

#include <d3d12.h>

namespace CYD
{
// =================================================================================================
// Implementation
class D3D12RenderBackendImp
{
  public:
   D3D12RenderBackendImp( const Window& window )
       : m_window( window ),
         m_factory(),
         m_devices( m_factory ),
         m_mainDevice( m_devices.getMainDevice() )
   {
   }

   ~D3D12RenderBackendImp() = default;

   void cleanup() const {}

   void waitUntilIdle() const {};

   CmdListHandle getMainCommandList() const { return {}; }

   CmdListHandle
   createCommandList( QueueUsageFlag usage, const std::string_view name, bool presentable )
   {
      return {};
   }

   void submitCommandList( CmdListHandle cmdList ) const {}

   void resetCommandList( CmdListHandle cmdList ) const {}

   void waitOnCommandList( CmdListHandle cmdList ) const {}

   void syncOnCommandList( CmdListHandle from, CmdListHandle to ) {}

   void destroyCommandList( CmdListHandle cmdList ) {}

   void syncOnSwapchain( CmdListHandle cmdList ) {}

   void syncToSwapchain( CmdListHandle cmdList ) {}

   void setViewport( CmdListHandle cmdList, const Viewport& viewport ) const {}

   void setScissor( CmdListHandle cmdList, const Rectangle& scissor ) const {}

   void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ) const {}

   void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo ) const {}

   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) const {}

   void bindIndexBuffer(
       CmdListHandle cmdList,
       IndexBufferHandle bufferHandle,
       IndexType type,
       uint32_t offset ) const
   {
   }

   void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       uint32_t binding,
       uint32_t set ) const
   {
   }

   void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       const SamplerInfo& smapler,
       uint32_t binding,
       uint32_t set ) const
   {
   }

   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t binding, uint32_t set )
       const
   {
      if( !texHandle )
      {
         CYD_ASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }
   }

   void bindBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t binding,
       uint32_t set,
       uint32_t offset,
       uint32_t range ) const
   {
   }

   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t binding,
       uint32_t set,
       uint32_t offset,
       uint32_t range ) const
   {
   }

   void updateConstantBuffer(
       CmdListHandle cmdList,
       PipelineStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData ) const
   {
   }

   TextureHandle createTexture( const TextureDescription& desc ) { return {}; }

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       uint32_t layerCount,
       const void* const* ppTexels )
   {
      return {};
   }

   TextureHandle
   createTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels )
   {
      return {};
   }

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices,
       const std::string_view name )
   {
      return {};
   }

   IndexBufferHandle createIndexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       const void* pIndices,
       const std::string_view name )
   {
      return {};
   }

   BufferHandle createUniformBuffer( size_t size, const std::string_view name ) { return {}; }

   BufferHandle createBuffer( size_t size, const std::string_view name ) { return {}; }

   void copyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size )
       const
   {
   }

   void destroyTexture( TextureHandle texHandle ) {}

   void destroyVertexBuffer( VertexBufferHandle bufferHandle ) {}

   void destroyIndexBuffer( IndexBufferHandle bufferHandle ) {}

   void destroyBuffer( BufferHandle bufferHandle ) {}

   void prepareFrame() const {}

   void beginRendering( CmdListHandle cmdList ) const {}

   void beginRendering(
       CmdListHandle cmdList,
       const FramebufferInfo& targetsInfo,
       const std::vector<TextureHandle>& targets ) const
   {
   }

   void nextPass( CmdListHandle cmdList ) const {}

   void endRendering( CmdListHandle cmdList ) const {}

   void draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) const {}

   void drawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ) const {}

   void drawInstanced(
       CmdListHandle cmdList,
       size_t vertexCount,
       size_t instanceCount,
       size_t firstVertex,
       size_t firstInstance ) const {};

   void drawIndexedInstanced(
       CmdListHandle cmdList,
       size_t indexCount,
       size_t instanceCount,
       size_t firstIndex,
       size_t firstInstance ) const {};

   void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ ) const {}

   void presentFrame() const {}

  private:
   const Window& m_window;
   d3d12::Factory m_factory;
   d3d12::DeviceManager m_devices;

   d3d12::Device& m_mainDevice;

   HandleManager m_coreHandles;
};

// =================================================================================================
// Link
D3D12RenderBackend::D3D12RenderBackend( const Window& window )
    : _imp( new D3D12RenderBackendImp( window ) )
{
}

D3D12RenderBackend::~D3D12RenderBackend() { delete _imp; }

void D3D12RenderBackend::cleanup() { _imp->cleanup(); }
void D3D12RenderBackend::waitUntilIdle() { _imp->waitUntilIdle(); }

CmdListHandle D3D12RenderBackend::getMainCommandList() const { return _imp->getMainCommandList(); }

CmdListHandle D3D12RenderBackend::createCommandList(
    QueueUsageFlag usage,
    const std::string_view name,
    bool presentable )
{
   return _imp->createCommandList( usage, name, presentable );
}

void D3D12RenderBackend::submitCommandList( CmdListHandle cmdList )
{
   _imp->submitCommandList( cmdList );
}

void D3D12RenderBackend::resetCommandList( CmdListHandle cmdList )
{
   _imp->resetCommandList( cmdList );
}

void D3D12RenderBackend::waitOnCommandList( CmdListHandle cmdList )
{
   _imp->waitOnCommandList( cmdList );
}

void D3D12RenderBackend::syncOnCommandList( CmdListHandle from, CmdListHandle to )
{
   _imp->syncOnCommandList( from, to );
}

void D3D12RenderBackend::destroyCommandList( CmdListHandle cmdList )
{
   _imp->destroyCommandList( cmdList );
}

void D3D12RenderBackend::syncOnSwapchain( CmdListHandle cmdList )
{
   _imp->syncOnSwapchain( cmdList );
}

void D3D12RenderBackend::syncToSwapchain( CmdListHandle cmdList )
{
   _imp->syncToSwapchain( cmdList );
}

void D3D12RenderBackend::bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo )
{
   _imp->bindPipeline( cmdList, pipInfo );
}

void D3D12RenderBackend::bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo )
{
   _imp->bindPipeline( cmdList, pipInfo );
}

void D3D12RenderBackend::bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   _imp->bindVertexBuffer( cmdList, bufferHandle );
}

void D3D12RenderBackend::bindIndexBuffer(
    CmdListHandle cmdList,
    IndexBufferHandle bufferHandle,
    IndexType type,
    uint32_t offset )
{
   _imp->bindIndexBuffer( cmdList, bufferHandle, type, offset );
}

void D3D12RenderBackend::bindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindTexture( cmdList, texHandle, binding, set );
}

void D3D12RenderBackend::bindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const SamplerInfo& sampler,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindTexture( cmdList, texHandle, sampler, binding, set );
}

void D3D12RenderBackend::bindImage(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t binding,
    uint32_t set )
{
   _imp->bindImage( cmdList, texHandle, binding, set );
}

void D3D12RenderBackend::bindBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set,
    uint32_t offset,
    uint32_t range )
{
   _imp->bindBuffer( cmdList, bufferHandle, binding, set, offset, range );
}

void D3D12RenderBackend::bindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set,
    uint32_t offset,
    uint32_t range )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle, binding, set, offset, range );
}

void D3D12RenderBackend::setViewport( CmdListHandle cmdList, const Viewport& viewport )
{
   _imp->setViewport( cmdList, viewport );
}

void D3D12RenderBackend::setScissor( CmdListHandle cmdList, const Rectangle& scissor )
{
   _imp->setScissor( cmdList, scissor );
}

void D3D12RenderBackend::updateConstantBuffer(
    CmdListHandle cmdList,
    PipelineStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData )
{
   _imp->updateConstantBuffer( cmdList, stages, offset, size, pData );
}

TextureHandle D3D12RenderBackend::createTexture( const CYD::TextureDescription& desc )
{
   return _imp->createTexture( desc );
}

TextureHandle D3D12RenderBackend::createTexture(
    CmdListHandle transferList,
    const CYD::TextureDescription& desc,
    const void* pTexels )
{
   return _imp->createTexture( transferList, desc, pTexels );
}

TextureHandle D3D12RenderBackend::createTexture(
    CmdListHandle transferList,
    const CYD::TextureDescription& desc,
    uint32_t layerCount,
    const void* const* ppTexels )
{
   return _imp->createTexture( transferList, desc, layerCount, ppTexels );
}

VertexBufferHandle D3D12RenderBackend::createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices,
    const std::string_view name )
{
   return _imp->createVertexBuffer( transferList, count, stride, pVertices, name );
}

IndexBufferHandle D3D12RenderBackend::createIndexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    const void* pIndices,
    const std::string_view name )
{
   return _imp->createIndexBuffer( transferList, count, pIndices, name );
}

BufferHandle D3D12RenderBackend::createUniformBuffer( size_t size, const std::string_view name )
{
   return _imp->createUniformBuffer( size, name );
}

BufferHandle D3D12RenderBackend::createBuffer( size_t size, const std::string_view name )
{
   return _imp->createBuffer( size, name );
}

void D3D12RenderBackend::copyToBuffer(
    BufferHandle bufferHandle,
    const void* pData,
    size_t offset,
    size_t size )
{
   _imp->copyToBuffer( bufferHandle, pData, offset, size );
}

void D3D12RenderBackend::destroyTexture( TextureHandle texHandle )
{
   _imp->destroyTexture( texHandle );
}

void D3D12RenderBackend::destroyVertexBuffer( VertexBufferHandle bufferHandle )
{
   _imp->destroyVertexBuffer( bufferHandle );
}

void D3D12RenderBackend::destroyIndexBuffer( IndexBufferHandle bufferHandle )
{
   _imp->destroyIndexBuffer( bufferHandle );
}

void D3D12RenderBackend::destroyBuffer( BufferHandle bufferHandle )
{
   _imp->destroyBuffer( bufferHandle );
}

void D3D12RenderBackend::prepareFrame() { _imp->prepareFrame(); }

void D3D12RenderBackend::beginRendering( CmdListHandle cmdList )
{
   _imp->beginRendering( cmdList );
}

void D3D12RenderBackend::beginRendering(
    CmdListHandle cmdList,
    const FramebufferInfo& targetsInfo,
    const std::vector<TextureHandle>& targets )
{
   _imp->beginRendering( cmdList, targetsInfo, targets );
}

void D3D12RenderBackend::nextPass( CmdListHandle cmdList ) { _imp->nextPass( cmdList ); }

void D3D12RenderBackend::endRendering( CmdListHandle cmdList ) { _imp->endRendering( cmdList ); }

void D3D12RenderBackend::draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex )
{
   _imp->draw( cmdList, vertexCount, firstVertex );
}

void D3D12RenderBackend::drawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex )
{
   _imp->drawIndexed( cmdList, indexCount, firstIndex );
}

void D3D12RenderBackend::drawInstanced(
    CmdListHandle cmdList,
    size_t vertexCount,
    size_t instanceCount,
    size_t firstVertex,
    size_t firstInstance )
{
   _imp->drawInstanced( cmdList, vertexCount, instanceCount, firstVertex, firstInstance );
}

void D3D12RenderBackend::drawIndexedInstanced(
    CmdListHandle cmdList,
    size_t indexCount,
    size_t instanceCount,
    size_t firstIndex,
    size_t firstInstance )
{
   _imp->drawIndexedInstanced( cmdList, indexCount, instanceCount, firstIndex, firstInstance );
}

void D3D12RenderBackend::dispatch(
    CmdListHandle cmdList,
    uint32_t workX,
    uint32_t workY,
    uint32_t workZ )
{
   _imp->dispatch( cmdList, workX, workY, workZ );
}

void D3D12RenderBackend::presentFrame() { _imp->presentFrame(); }
}