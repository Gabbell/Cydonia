#include <Graphics/GRIS/Backends/VKRenderBackend.h>

#include <Common/Assert.h>

#include <Window/GLFWWindow.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Utility/GraphicsIO.h>
#include <Graphics/Handles/ResourceHandleManager.h>
#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/DeviceHerder.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/BarriersHelper.h>

#include <unordered_map>
#include <vector>

namespace CYD
{
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
      // Initializing swapchain on main device
      SwapchainInfo scInfo = {};
      scInfo.extent        = window.getExtent();
      scInfo.format        = PixelFormat::BGRA8_UNORM;
      scInfo.space         = ColorSpace::SRGB_NONLINEAR;
      scInfo.mode          = PresentMode::IMMEDIATE;

      m_mainDevice    = m_devices.getMainDevice();
      m_mainSwapchain = m_mainDevice->createSwapchain( scInfo );
   }

   ~VKRenderBackendImp() = default;

   void cleanup() const { m_mainDevice->cleanup(); }

   CmdListHandle createCommandList( QueueUsageFlag usage, bool presentable )
   {
      const auto cmdBuffer = m_mainDevice->createCommandBuffer( usage, presentable );
      return m_coreHandles.add( cmdBuffer, HandleType::CMDLIST );
   }

   void startRecordingCommandList( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->startRecording();
   }

   void endRecordingCommandList( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->endRecording();
   }

   void submitCommandList( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->submit();
   }

   void resetCommandList( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->reset();
   }

   void waitOnCommandList( CmdListHandle cmdList ) const
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
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
            buffer->decUse();
         }

         m_cmdListDeps.erase( it );
      }

      m_coreHandles.remove( cmdList );
   }

   void setViewport( CmdListHandle cmdList, const Viewport& viewport ) const
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->setViewport( viewport );
   }

   void setScissor( CmdListHandle cmdList, const Rectangle& scissor ) const
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->setScissor( scissor );
   }

   void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->bindPipeline( pipInfo );
   }

   void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->bindPipeline( pipInfo );
   }

   void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) const
   {
      if( !bufferHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid vertex buffer" );
         return;
      }

      const auto cmdBuffer    = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto vertexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindVertexBuffer( vertexBuffer );
   }

   void bindIndexBuffer( CmdListHandle cmdList, IndexBufferHandle bufferHandle, IndexType type )
       const
   {
      if( !bufferHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid index buffer" );
         return;
      }

      const auto cmdBuffer   = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto indexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindIndexBuffer( indexBuffer, type );
   }

   void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       uint32_t set,
       uint32_t binding ) const
   {
      if( !texHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindTexture( texture, set, binding );
   }

   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding )
       const
   {
      if( !texHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindImage( texture, set, binding );
   }

   void bindBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding ) const
   {
      auto cmdBuffer    = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindBuffer( buffer, set, binding );
   }

   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t set,
       uint32_t binding ) const
   {
      auto cmdBuffer           = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto uniformBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindUniformBuffer( uniformBuffer, set, binding );
   }

   void bindTexture( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name )
       const
   {
      if( !texHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindTexture( texture, name );
   }

   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, const std::string_view name )
       const
   {
      if( !texHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindImage( texture, name );
   }

   void bindBuffer( CmdListHandle cmdList, BufferHandle bufferHandle, const std::string_view name )
       const
   {
      auto cmdBuffer    = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindBuffer( buffer, name );
   }

   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       const std::string_view name ) const
   {
      auto cmdBuffer           = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto uniformBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindUniformBuffer( uniformBuffer, name );
   }

   void updateConstantBuffer(
       CmdListHandle cmdList,
       ShaderStageFlag stages,
       size_t offset,
       size_t size,
       const void* pData ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      PushConstantRange range = {};
      range.stages            = stages;
      range.offset            = offset;
      range.size              = size;
      cmdBuffer->updatePushConstants( range, pData );
   }

   TextureHandle createTexture( CmdListHandle transferList, const TextureDescription& desc )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // Creating GPU texture
      vk::Texture* texture = m_mainDevice->createTexture( desc );

      if( desc.stages == ShaderStage::FRAGMENT_STAGE )
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::SHADER_READ );
      }
      else
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::GENERAL );
      }

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const std::string& path )
   {
      void* imageData = GraphicsIO::LoadImage( desc, path );

      if( !imageData )
      {
         return Handle();
      }

      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // Staging
      vk::Buffer* staging = m_mainDevice->createStagingBuffer( desc.size );
      staging->copy( imageData, 0, desc.size );
      GraphicsIO::FreeImage( imageData );

      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice->createTexture( desc );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::TRANSFER_DST );

      cmdBuffer->uploadBufferToTex( staging, texture );

      if( desc.stages == ShaderStage::FRAGMENT_STAGE )
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::SHADER_READ );
      }
      else
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::GENERAL );
      }

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       const std::vector<std::string>& paths )
   {
      CYDASSERT(
          paths.size() <= desc.layers &&
          "VKRenderBackend:: Number of textures could not fit in number of layers " );

      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      vk::Buffer* staging = m_mainDevice->createStagingBuffer( desc.size );

      const size_t layerSize = desc.size / desc.layers;

      for( uint32_t i = 0; i < paths.size(); ++i )
      {
         void* imageData = GraphicsIO::LoadImage( desc, paths[i] );

         if( !imageData )
         {
            return Handle();
         }

         staging->copy( imageData, i * layerSize, layerSize );
         GraphicsIO::FreeImage( imageData );
      }

      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice->createTexture( desc );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::TRANSFER_DST );

      cmdBuffer->uploadBufferToTex( staging, texture );

      if( desc.stages == ShaderStage::FRAGMENT_STAGE )
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::SHADER_READ );
      }
      else
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::GENERAL );
      }

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   TextureHandle
   createTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // Staging
      vk::Buffer* staging = m_mainDevice->createStagingBuffer( desc.size );
      staging->copy( pTexels, 0, desc.size );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice->createTexture( desc );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::TRANSFER_DST );

      cmdBuffer->uploadBufferToTex( staging, texture );

      if( desc.stages == ShaderStage::FRAGMENT_STAGE )
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::SHADER_READ );
      }
      else
      {
         vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::GENERAL );
      }

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      const size_t bufferSize = static_cast<size_t>( count ) * stride;

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
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // TODO Dynamic uint32_t/uint16_t
      const size_t bufferSize = count * sizeof( uint32_t );

      // Staging
      vk::Buffer* staging = m_mainDevice->createStagingBuffer( bufferSize );
      staging->copy( pIndices, 0, bufferSize );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* indexBuffer = m_mainDevice->createIndexBuffer( bufferSize );
      cmdBuffer->copyBuffer( staging, indexBuffer );

      return m_coreHandles.add( indexBuffer, HandleType::INDEXBUFFER );
   }

   BufferHandle createUniformBuffer( size_t size )
   {
      const auto uniformBuffer = m_mainDevice->createUniformBuffer( size );

      return m_coreHandles.add( uniformBuffer, HandleType::BUFFER );
   }

   BufferHandle createBuffer( size_t size )
   {
      const auto deviceBuffer = m_mainDevice->createBuffer( size );

      return m_coreHandles.add( deviceBuffer, HandleType::BUFFER );
   }

   void copyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size )
       const
   {
      if( bufferHandle )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->copy( pData, offset, size );
      }
   }

   void destroyTexture( TextureHandle texHandle )
   {
      if( texHandle )
      {
         m_coreHandles.remove( texHandle );
      }
   }

   void destroyVertexBuffer( VertexBufferHandle bufferHandle )
   {
      if( bufferHandle )
      {
         m_coreHandles.remove( bufferHandle );
      }
   }

   void destroyIndexBuffer( IndexBufferHandle bufferHandle )
   {
      if( bufferHandle )
      {
         m_coreHandles.remove( bufferHandle );
      }
   }

   void destroyBuffer( BufferHandle bufferHandle )
   {
      if( bufferHandle )
      {
         m_coreHandles.remove( bufferHandle );
      }
   }

   void prepareFrame() const {}

   void beginRendering( CmdListHandle cmdList, bool wantDepth ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->beginRendering( *m_mainSwapchain, wantDepth );
   }

   void beginRendering(
       CmdListHandle cmdList,
       const RenderTargetsInfo& targetsInfo,
       const std::vector<TextureHandle>& targets ) const
   {
      // Fetching textures
      std::vector<const vk::Texture*> vkTextures;
      vkTextures.reserve( targets.size() );
      for( const auto& texture : targets )
      {
         const auto vkTexture = static_cast<vk::Texture*>( m_coreHandles.get( texture ) );
         vkTextures.push_back( vkTexture );
      }

      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->beginRendering( targetsInfo, vkTextures );
   }

   void nextPass( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->nextPass();
   }

   void endRendering( CmdListHandle cmdList ) const
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->endRendering();
   }

   void drawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->draw( vertexCount, firstVertex );
   }

   void drawVerticesIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->drawIndexed( indexCount, firstIndex );
   }

   void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->dispatch( workX, workY, workZ );
   }

   void presentFrame() const { m_mainSwapchain->present(); }

  private:
   vk::Instance m_instance;
   vk::Surface m_surface;
   vk::DeviceHerder m_devices;

   vk::Device* m_mainDevice       = nullptr;
   vk::Swapchain* m_mainSwapchain = nullptr;

   HandleManager m_coreHandles;

   // Used to store intermediate buffers like staging buffers to be able to flag them as unused once
   // the command list is getting destroyed
   using CmdListDependencyMap = std::unordered_map<uint32_t, std::vector<vk::Buffer*>>;
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

void VKRenderBackend::bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo )
{
   _imp->bindPipeline( cmdList, pipInfo );
}

void VKRenderBackend::bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo )
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

void VKRenderBackend::bindImage(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindImage( cmdList, texHandle, set, binding );
}

void VKRenderBackend::bindBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindBuffer( cmdList, bufferHandle, set, binding );
}

void VKRenderBackend::bindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t set,
    uint32_t binding )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle, set, binding );
}

void VKRenderBackend::bindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const std::string_view name )
{
   _imp->bindTexture( cmdList, texHandle, name );
}

void VKRenderBackend::bindImage(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const std::string_view name )
{
   _imp->bindImage( cmdList, texHandle, name );
}

void VKRenderBackend::bindBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    const std::string_view name )
{
   _imp->bindBuffer( cmdList, bufferHandle, name );
}

void VKRenderBackend::bindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    const std::string_view name )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle, name );
}

void VKRenderBackend::setViewport( CmdListHandle cmdList, const Viewport& viewport )
{
   _imp->setViewport( cmdList, viewport );
}

void VKRenderBackend::setScissor( CmdListHandle cmdList, const Rectangle& scissor )
{
   _imp->setScissor( cmdList, scissor );
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
    const CYD::TextureDescription& desc )
{
   return _imp->createTexture( transferList, desc );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const CYD::TextureDescription& desc,
    const std::string& path )
{
   return _imp->createTexture( transferList, desc, path );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    const std::vector<std::string>& paths )
{
   return _imp->createTexture( transferList, desc, paths );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const CYD::TextureDescription& desc,
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

BufferHandle VKRenderBackend::createUniformBuffer( size_t size )
{
   return _imp->createUniformBuffer( size );
}

BufferHandle VKRenderBackend::createBuffer( size_t size ) { return _imp->createBuffer( size ); }

void VKRenderBackend::copyToBuffer(
    BufferHandle bufferHandle,
    const void* pData,
    size_t offset,
    size_t size )
{
   _imp->copyToBuffer( bufferHandle, pData, offset, size );
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

void VKRenderBackend::destroyBuffer( BufferHandle bufferHandle )
{
   _imp->destroyBuffer( bufferHandle );
}

void VKRenderBackend::prepareFrame() { _imp->prepareFrame(); }

void VKRenderBackend::beginRendering( CmdListHandle cmdList, bool wantDepth )
{
   _imp->beginRendering( cmdList, wantDepth );
}

void VKRenderBackend::beginRendering(
    CmdListHandle cmdList,
    const RenderTargetsInfo& targetsInfo,
    const std::vector<TextureHandle>& targets )
{
   _imp->beginRendering( cmdList, targetsInfo, targets );
}

void VKRenderBackend::nextPass( CmdListHandle cmdList ) { _imp->nextPass( cmdList ); }

void VKRenderBackend::endRendering( CmdListHandle cmdList ) { _imp->endRendering( cmdList ); }

void VKRenderBackend::drawVertices( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex )
{
   _imp->drawVertices( cmdList, vertexCount, firstVertex );
}

void VKRenderBackend::drawVerticesIndexed(
    CmdListHandle cmdList,
    size_t indexCount,
    size_t firstIndex )
{
   _imp->drawVerticesIndexed( cmdList, indexCount, firstIndex );
}

void VKRenderBackend::dispatch(
    CmdListHandle cmdList,
    uint32_t workX,
    uint32_t workY,
    uint32_t workZ )
{
   _imp->dispatch( cmdList, workX, workY, workZ );
}

void VKRenderBackend::presentFrame() { _imp->presentFrame(); }
}
