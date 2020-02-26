#include <Graphics/Backends/VKRenderBackend.h>

#include <Window/GLFWWindow.h>

#include <Graphics/GraphicsTypes.h>
#include <Handles/HandleManager.h>
#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/DeviceHerder.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>

#include <unordered_map>
#include <vector>

namespace cyd
{
using CmdListDependencyMap = std::unordered_map<uint32_t, std::vector<vk::Buffer*>>;

// =================================================================================================
// Implementation
class VKRenderBackendImp
{
  public:
   VKRenderBackendImp( const Window& window )
       : m_instance( window ),
         m_surface( window, m_instance ),
         m_devices( window, m_instance, m_surface )
   {
      SwapchainInfo scInfo = {};
      scInfo.extent        = window.getExtent();
      scInfo.format        = PixelFormat::BGRA8_UNORM;
      scInfo.space         = ColorSpace::SRGB_NONLINEAR;
      scInfo.mode          = PresentMode::MAILBOX;

      m_mainDevice    = m_devices.getMainDevice();
      m_mainSwapchain = m_mainDevice->createSwapchain( scInfo );
   }

   ~VKRenderBackendImp() = default;

   void cleanup() { m_mainDevice->cleanup(); }

   CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable )
   {
      auto cmdBuffer = m_mainDevice->createCommandBuffer( usage, presentable );
      return m_coreHandles.add( cmdBuffer, HandleType::CMDLIST );
   }

   void startRecordingCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->startRecording();
   }

   void endRecordingCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->endRecording();
   }

   void submitCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->submit();
   }

   void resetCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->reset();
   }

   void waitOnCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->waitForCompletion();
   }

   void destroyCommandList( CmdListHandle cmdList )
   {
      auto it = m_cmdListDeps.find( cmdList );
      if( it != m_cmdListDeps.end() )
      {
         // This command list has buffer dependencies that need to be flagged as unused
         for( auto& buffer : it->second )
         {
            buffer->setUnused();
         }

         m_cmdListDeps.erase( it );
      }

      m_coreHandles.remove( cmdList );
   }

   void setViewport( CmdListHandle cmdList, const Rectangle& viewport )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->setViewport( viewport );
   }

   void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->bindPipeline( pipInfo );
   }

   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
   {
      auto cmdBuffer    = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto vertexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindVertexBuffer( vertexBuffer );
   }

   void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle, IndexType type )
   {
      auto cmdBuffer   = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto indexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindIndexBuffer( indexBuffer, type );
   }

   void
   bindTexture( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto texture   = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindTexture( texture, set, binding );
   }

   void bindUniformBuffer(
       CmdListHandle cmdList,
       UniformBufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding )
   {
      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto uniformBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindBuffer( uniformBuffer, set, binding );
   }

   void updateConstantBuffer(
       CmdListHandle cmdList,
       ShaderStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      PushConstantRange range = {};
      range.stages            = stages;
      range.offset            = offset;
      range.size              = size;
      cmdBuffer->updatePushConstants( range, pData );
   }

   TextureHandle
   createTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // Staging
      vk::Buffer* staging = m_mainDevice->createStagingBuffer( desc.size );
      staging->copy( pTexels, 0, desc.size );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice->createTexture( desc );
      cmdBuffer->uploadBufferToTex( staging, texture );

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      size_t bufferSize = static_cast<size_t>( count ) * stride;

      // Staging
      vk::Buffer* staging = m_mainDevice->createStagingBuffer( bufferSize );
      staging->copy( pVertices, 0, bufferSize );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* vertexBuffer = m_mainDevice->createVertexBuffer( bufferSize );
      cmdBuffer->copyBuffer( staging, vertexBuffer );

      return m_coreHandles.add( vertexBuffer, HandleType::VERTEXBUFFER );
   }

   IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* pIndices )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // TODO Dynamic uint32_t/uint16_t
      size_t bufferSize = count * sizeof( uint32_t );

      // Staging
      vk::Buffer* staging = m_mainDevice->createStagingBuffer( bufferSize );
      staging->copy( pIndices, 0, bufferSize );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* indexBuffer = m_mainDevice->createIndexBuffer( bufferSize );
      cmdBuffer->copyBuffer( staging, indexBuffer );

      return m_coreHandles.add( indexBuffer, HandleType::INDEXBUFFER );
   }

   UniformBufferHandle createUniformBuffer( size_t size )
   {
      auto uniformBuffer = m_mainDevice->createUniformBuffer( size );

      return m_coreHandles.add( uniformBuffer, HandleType::UNIFORMBUFFER );
   }

   void copyToUniformBuffer(
       UniformBufferHandle bufferHandle,
       const void* pData,
       size_t offset,
       size_t size )
   {
      if( bufferHandle != Handle::INVALID_HANDLE )
      {
         auto uniformBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         uniformBuffer->copy( pData, offset, size );
      }
   }

   void destroyTexture( TextureHandle texHandle )
   {
      if( texHandle != Handle::INVALID_HANDLE )
      {
         auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );
         texture->setUnused();

         m_coreHandles.remove( texHandle );
      }
   }

   void destroyVertexBuffer( VertexBufferHandle bufferHandle )
   {
      if( bufferHandle != Handle::INVALID_HANDLE )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->setUnused();

         m_coreHandles.remove( bufferHandle );
      }
   }

   void destroyIndexBuffer( IndexBufferHandle bufferHandle )
   {
      if( bufferHandle != Handle::INVALID_HANDLE )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->setUnused();

         m_coreHandles.remove( bufferHandle );
      }
   }

   void destroyUniformBuffer( UniformBufferHandle bufferHandle )
   {
      if( bufferHandle != Handle::INVALID_HANDLE )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->setUnused();

         m_coreHandles.remove( bufferHandle );
      }
   }

   void beginRenderSwapchain( CmdListHandle cmdList, const RenderPassInfo& renderPassInfo )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->beginPass( renderPassInfo, *m_mainSwapchain );
   }

   void endRenderPass( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->endPass();
   }

   void drawVertices( CmdListHandle cmdList, uint32_t vertexCount )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->draw( vertexCount );
   }

   void drawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->drawIndexed( indexCount );
   }

   void presentFrame() { m_mainSwapchain->present(); }

  private:
   vk::Instance m_instance;
   vk::Surface m_surface;
   vk::DeviceHerder m_devices;

   vk::Device* m_mainDevice       = nullptr;
   vk::Swapchain* m_mainSwapchain = nullptr;

   HandleManager m_coreHandles;

   // Used to store intermediate buffers like staging buffers to be able to flag them as unused once
   // the command list is getting destroyed
   CmdListDependencyMap m_cmdListDeps;
};

// =================================================================================================
// Link
VKRenderBackend::VKRenderBackend( const Window& window ) : _imp( new VKRenderBackendImp( window ) )
{
}

VKRenderBackend::~VKRenderBackend() { delete _imp; }

void VKRenderBackend::cleanup() { _imp->cleanup(); }

CmdListHandle VKRenderBackend::createCommandList( QueueUsageFlag usage, bool presentable )
{
   return _imp->createCommandList( usage, presentable );
}

void VKRenderBackend::startRecordingCommandList( CmdListHandle cmdList )
{
   _imp->startRecordingCommandList( cmdList );
}

void VKRenderBackend::endRecordingCommandList( CmdListHandle cmdList )
{
   _imp->endRecordingCommandList( cmdList );
}

void VKRenderBackend::submitCommandList( CmdListHandle cmdList )
{
   _imp->submitCommandList( cmdList );
}

void VKRenderBackend::resetCommandList( CmdListHandle cmdList )
{
   _imp->resetCommandList( cmdList );
}

void VKRenderBackend::waitOnCommandList( CmdListHandle cmdList )
{
   _imp->waitOnCommandList( cmdList );
}

void VKRenderBackend::destroyCommandList( CmdListHandle cmdList )
{
   return _imp->destroyCommandList( cmdList );
}

void VKRenderBackend::bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )
{
   _imp->bindPipeline( cmdList, pipInfo );
}

void VKRenderBackend::bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   _imp->bindVertexBuffer( cmdList, bufferHandle );
}

void VKRenderBackend::bindIndexBuffer(
    CmdListHandle cmdList,
    IndexBufferHandle bufferHandle,
    IndexType type )
{
   _imp->bindIndexBuffer( cmdList, bufferHandle, type );
}

void VKRenderBackend::bindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindTexture( cmdList, texHandle, set, binding );
}

void VKRenderBackend::bindUniformBuffer(
    CmdListHandle cmdList,
    UniformBufferHandle bufferHandle,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle, set, binding );
}

void VKRenderBackend::setViewport( CmdListHandle cmdList, const Rectangle& viewport )
{
   _imp->setViewport( cmdList, viewport );
}

void VKRenderBackend::updateConstantBuffer(
    CmdListHandle cmdList,
    ShaderStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData )
{
   _imp->updateConstantBuffer( cmdList, stages, offset, size, pData );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const cyd::TextureDescription& desc,
    const void* pTexels )
{
   return _imp->createTexture( transferList, desc, pTexels );
}

VertexBufferHandle VKRenderBackend::createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices )
{
   return _imp->createVertexBuffer( transferList, count, stride, pVertices );
}

IndexBufferHandle VKRenderBackend::createIndexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    const void* pIndices )
{
   return _imp->createIndexBuffer( transferList, count, pIndices );
}

UniformBufferHandle VKRenderBackend::createUniformBuffer( size_t size )
{
   return _imp->createUniformBuffer( size );
}

void VKRenderBackend::copyToUniformBuffer(
    UniformBufferHandle bufferHandle,
    const void* pData,
    size_t offset,
    size_t size )
{
   _imp->copyToUniformBuffer( bufferHandle, pData, offset, size );
}

void VKRenderBackend::destroyTexture( TextureHandle texHandle )
{
   _imp->destroyTexture( texHandle );
}

void VKRenderBackend::destroyVertexBuffer( VertexBufferHandle bufferHandle )
{
   _imp->destroyVertexBuffer( bufferHandle );
}

void VKRenderBackend::destroyIndexBuffer( IndexBufferHandle bufferHandle )
{
   _imp->destroyIndexBuffer( bufferHandle );
}

void VKRenderBackend::destroyUniformBuffer( UniformBufferHandle bufferHandle )
{
   _imp->destroyUniformBuffer( bufferHandle );
}

void VKRenderBackend::beginRenderSwapchain(
    CmdListHandle cmdList,
    const RenderPassInfo& renderPassInfo )
{
   _imp->beginRenderSwapchain( cmdList, renderPassInfo );
}

void VKRenderBackend::endRenderPass( CmdListHandle cmdList ) { _imp->endRenderPass( cmdList ); }

void VKRenderBackend::drawVertices( CmdListHandle cmdList, uint32_t vertexCount )
{
   _imp->drawVertices( cmdList, vertexCount );
}

void VKRenderBackend::drawVerticesIndexed( CmdListHandle cmdList, uint32_t indexCount )
{
   _imp->drawVerticesIndexed( cmdList, indexCount );
}

void VKRenderBackend::presentFrame() { _imp->presentFrame(); }
}
