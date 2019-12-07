#include <Graphics/Backends/VKRenderBackend.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>

#include <Window/GLFWWindow.h>

#include <Handles/HandleManager.h>

#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/DeviceHerder.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/CommandBuffer.h>

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
       : _instance( window ), _surface( window, _instance ), _devices( window, _instance, _surface )
   {
      SwapchainInfo scInfo = {};
      scInfo.extent        = window.getExtent();
      scInfo.format        = PixelFormat::BGRA8_UNORM;
      scInfo.space         = ColorSpace::SRGB_NONLINEAR;
      scInfo.mode          = PresentMode::MAILBOX;

      _mainDevice    = _devices.getMainDevice();
      _mainSwapchain = _mainDevice->createSwapchain( scInfo );
   }

   ~VKRenderBackendImp() {}

   void cleanup() { _mainDevice->cleanup(); }

   CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable )
   {
      auto cmdBuffer = _mainDevice->createCommandBuffer( usage, presentable );
      return _handles.add( cmdBuffer, HandleType::CMDLIST );
   }

   void startRecordingCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->startRecording();
   }

   void endRecordingCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->endRecording();
   }

   void submitCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->submit();
   }

   void waitOnCommandList( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->waitForCompletion();
   }

   void destroyCommandList( CmdListHandle cmdList )
   {
      auto it = _cmdListDeps.find( cmdList );
      if( it != _cmdListDeps.end() )
      {
         // This command list has buffer dependencies that need to be flagged as unused
         for( auto& buffer : it->second )
         {
            buffer->setUnused();
         }

         _cmdListDeps.erase( it );
      }

      _handles.remove( cmdList );
   }

   void bindPipeline( CmdListHandle cmdList, const PipelineInfo& pipInfo )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->bindPipeline( pipInfo );
   }

   void bindTexture( CmdListHandle cmdList, TextureHandle texHandle )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      auto texture   = static_cast<vk::Texture*>( _handles.get( texHandle ) );

      cmdBuffer->bindTexture( texture );
   }

   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
   {
      auto cmdBuffer    = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      auto vertexBuffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );

      cmdBuffer->bindVertexBuffer( vertexBuffer );
   }

   void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
   {
      auto cmdBuffer   = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      auto indexBuffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );

      cmdBuffer->bindIndexBuffer<uint32_t>( indexBuffer );
   }

   void bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle )
   {
      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      auto uniformBuffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );

      cmdBuffer->bindBuffer( uniformBuffer );
   }

   void setViewport( CmdListHandle cmdList, const Rectangle& viewport )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->setViewport( viewport );
   }

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const ShaderObjectInfo& info,
       const DescriptorSetLayoutInfo& layout,
       const void* texels )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( transferList ) );

      // Staging
      vk::Buffer* staging = _mainDevice->createStagingBuffer( desc.size );
      staging->mapMemory( texels );
      _cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = _mainDevice->createTexture( desc, info, layout );
      cmdBuffer->uploadBufferToTex( staging, texture );

      return _handles.add( texture, HandleType::TEXTURE );
   }

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* vertices )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( transferList ) );

      size_t bufferSize = static_cast<size_t>( count ) * stride;

      // Staging
      vk::Buffer* staging = _mainDevice->createStagingBuffer( bufferSize );
      staging->mapMemory( vertices );
      _cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* vertexBuffer = _mainDevice->createVertexBuffer( bufferSize );
      cmdBuffer->copyBuffer( staging, vertexBuffer );

      return _handles.add( vertexBuffer, HandleType::VERTEXBUFFER );
   }

   IndexBufferHandle
   createIndexBuffer( CmdListHandle transferList, uint32_t count, const void* indices )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( transferList ) );

      // TODO Dynamic uint32_t/uint16_t
      size_t bufferSize = count * sizeof( uint32_t );

      // Staging
      vk::Buffer* staging = _mainDevice->createStagingBuffer( bufferSize );
      staging->mapMemory( indices );
      _cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* indexBuffer = _mainDevice->createIndexBuffer( bufferSize );
      cmdBuffer->copyBuffer( staging, indexBuffer );

      return _handles.add( indexBuffer, HandleType::INDEXBUFFER );
   }

   UniformBufferHandle createUniformBuffer(
       size_t size,
       const ShaderObjectInfo& info,
       const DescriptorSetLayoutInfo& layout )
   {
      auto uniformBuffer = _mainDevice->createUniformBuffer( size, info, layout );

      return _handles.add( uniformBuffer, HandleType::UNIFORMBUFFER );
   }

   void mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* data )
   {
      auto uniformBuffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );
      uniformBuffer->mapMemory( data );
   }

   void destroyTexture( TextureHandle texHandle )
   {
      auto texture = static_cast<vk::Texture*>( _handles.get( texHandle ) );
      texture->setUnused();

      _handles.remove( texHandle );
   }

   void destroyVertexBuffer( VertexBufferHandle bufferHandle )
   {
      auto buffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );
      buffer->setUnused();

      _handles.remove( bufferHandle );
   }

   void destroyIndexBuffer( IndexBufferHandle bufferHandle )
   {
      auto buffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );
      buffer->setUnused();

      _handles.remove( bufferHandle );
   }

   void destroyUniformBuffer( UniformBufferHandle bufferHandle )
   {
      auto buffer = static_cast<vk::Buffer*>( _handles.get( bufferHandle ) );
      buffer->setUnused();

      _handles.remove( bufferHandle );
   }

   void beginRenderPass( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->beginPass( *_mainSwapchain );
   }

   void endRenderPass( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->endPass();
   }

   void drawFrame( CmdListHandle cmdList, uint32_t vertexCount )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( _handles.get( cmdList ) );
      cmdBuffer->draw( vertexCount );
   }

   void presentFrame() { _mainSwapchain->present(); }

  private:
   vk::Instance _instance;
   vk::Surface _surface;
   vk::DeviceHerder _devices;

   vk::Device* _mainDevice       = nullptr;
   vk::Swapchain* _mainSwapchain = nullptr;

   cyd::HandleManager _handles;

   // Used to store intermediate buffers like staging buffers to be able to flag them as unused once
   // the command list is getting destroyed
   CmdListDependencyMap _cmdListDeps;
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

void VKRenderBackend::bindTexture( CmdListHandle cmdList, TextureHandle texHandle )
{
   _imp->bindTexture( cmdList, texHandle );
}

void VKRenderBackend::bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   _imp->bindVertexBuffer( cmdList, bufferHandle );
}

void VKRenderBackend::bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle )
{
   _imp->bindIndexBuffer( cmdList, bufferHandle );
}

void VKRenderBackend::bindUniformBuffer( CmdListHandle cmdList, UniformBufferHandle bufferHandle )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle );
}

void VKRenderBackend::setViewport( CmdListHandle cmdList, const Rectangle& viewport )
{
   _imp->setViewport( cmdList, viewport );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const cyd::TextureDescription& desc,
    const cyd::ShaderObjectInfo& info,
    const cyd::DescriptorSetLayoutInfo& layout,
    const void* texels )
{
   return _imp->createTexture( transferList, desc, info, layout, texels );
}

VertexBufferHandle VKRenderBackend::createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* vertices )
{
   return _imp->createVertexBuffer( transferList, count, stride, vertices );
}

IndexBufferHandle VKRenderBackend::createIndexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    const void* indices )
{
   return _imp->createIndexBuffer( transferList, count, indices );
}

UniformBufferHandle VKRenderBackend::createUniformBuffer(
    size_t size,
    const ShaderObjectInfo& info,
    const DescriptorSetLayoutInfo& layout )
{
   return _imp->createUniformBuffer( size, info, layout );
}

void VKRenderBackend::mapUniformBufferMemory( UniformBufferHandle bufferHandle, const void* data )
{
   _imp->mapUniformBufferMemory( bufferHandle, data );
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

void VKRenderBackend::beginRenderPass( CmdListHandle cmdList ) { _imp->beginRenderPass( cmdList ); }

void VKRenderBackend::endRenderPass( CmdListHandle cmdList ) { _imp->endRenderPass( cmdList ); }

void VKRenderBackend::drawFrame( CmdListHandle cmdList, uint32_t vertexCount )
{
   _imp->drawFrame( cmdList, vertexCount );
}

// void drawFrameIndexed( CmdListHandle handle, uint32_t indexCount );

void VKRenderBackend::presentFrame() { _imp->presentFrame(); }
}
