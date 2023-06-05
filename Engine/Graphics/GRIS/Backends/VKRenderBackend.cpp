#include <Graphics/GRIS/Backends/VKRenderBackend.h>

#include <Common/Assert.h>

#include <Window/GLFWWindow.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/Handles/ResourceHandleManager.h>
#include <Graphics/Utility/GraphicsIO.h>
#include <Graphics/Vulkan/BarriersHelper.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/DebugUtilsLabel.h>
#include <Graphics/Vulkan/DescriptorPool.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/DeviceManager.h>
#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/SamplerCache.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <ThirdParty/ImGui/imgui_impl_glfw.h>
#include <ThirdParty/ImGui/imgui_impl_vulkan.h>

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
       : m_window( window ),
         m_instance( m_window ),
         m_surface( m_window, m_instance ),
         m_devices( m_window, m_instance, m_surface ),
         m_mainDevice( m_devices.getMainDevice() )
   {
      // Initializing swapchain on main device
      SwapchainInfo scInfo = {};
      scInfo.imageCount    = 2;  // Double buffered
      scInfo.extent        = window.getExtent();
      scInfo.format        = PixelFormat::BGRA8_UNORM;
      scInfo.space         = ColorSpace::SRGB_NONLINEAR;
      scInfo.mode          = PresentMode::IMMEDIATE;

      m_mainSwapchain = m_mainDevice.createSwapchain( scInfo );

      vk::DebugUtilsLabel::Initialize( m_mainDevice );
   }

   ~VKRenderBackendImp() = default;

   bool initializeUI()
   {
      // Initialize ImGui
      ImGui_ImplGlfw_InitForVulkan( m_window.getGLFWwindow(), true );

      const uint32_t graphicsFamilyIndex = m_mainDevice.getQueueFamilyIndexFromUsage( GRAPHICS );

      const vk::Device::QueueFamily& queueFamily =
          m_mainDevice.getQueueFamilyFromIndex( graphicsFamilyIndex );

      if( queueFamily.queues.empty() )
      {
         CYDASSERT( !"Could not find a graphics queue to initialize UI" );
         return false;
      }

      ImGui_ImplVulkan_InitInfo init_info = {};
      init_info.Instance                  = m_instance.getVKInstance();
      init_info.PhysicalDevice            = m_mainDevice.getPhysicalDevice();
      init_info.Device                    = m_mainDevice.getVKDevice();
      init_info.QueueFamily               = graphicsFamilyIndex;
      init_info.Queue                     = queueFamily.queues[0];
      init_info.PipelineCache             = nullptr;
      init_info.DescriptorPool            = m_mainDevice.getDescriptorPool().getVKDescriptorPool();
      init_info.Allocator                 = nullptr;
      init_info.MinImageCount             = m_mainSwapchain->getImageCount();
      init_info.ImageCount                = m_mainSwapchain->getImageCount();
      init_info.CheckVkResultFn           = nullptr;

      ImGui_ImplVulkan_Init( &init_info, m_mainSwapchain->getRenderPass() );

      // Loading fonts
      const auto cmdBuffer = m_mainDevice.createCommandBuffer( GRAPHICS, "ImGui Font Loading" );

      cmdBuffer->startRecording();

      ImGui_ImplVulkan_CreateFontsTexture( cmdBuffer->getVKBuffer() );

      cmdBuffer->endRecording();
      cmdBuffer->submit();
      cmdBuffer->waitForCompletion();

      ImGui_ImplVulkan_DestroyFontUploadObjects();

      m_hasUI = true;

      return true;
   }

   void uninitializeUI()
   {
      ImGui_ImplVulkan_Shutdown();
      ImGui_ImplGlfw_Shutdown();

      m_hasUI = false;
   }

   void drawUI( CmdListHandle cmdList ) const
   {
      ImGui::Render();

      ImDrawData* drawData = ImGui::GetDrawData();

      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      ImGui_ImplVulkan_RenderDrawData( drawData, cmdBuffer->getVKBuffer() );
   }

   void cleanup() const { m_mainDevice.cleanup(); }

   void waitUntilIdle() const { m_mainDevice.waitUntilIdle(); }

   CmdListHandle
   createCommandList( QueueUsageFlag usage, const std::string_view name, bool presentable )
   {
      const auto cmdBuffer = m_mainDevice.createCommandBuffer( usage, name, presentable );
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

   void syncOnCommandList( CmdListHandle from, CmdListHandle to )
   {
      const auto fromCmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( from ) );
      auto toCmdBuffer         = static_cast<vk::CommandBuffer*>( m_coreHandles.get( to ) );

      toCmdBuffer->syncOnCommandList( fromCmdBuffer );
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

      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->decUse();

      m_coreHandles.remove( cmdList );
   }

   void syncOnSwapchain( CmdListHandle cmdList )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->syncOnSwapchain( m_mainSwapchain );
   }

   void syncToSwapchain( CmdListHandle cmdList )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->syncToSwapchain( m_mainSwapchain );
   }

   void setViewport( CmdListHandle cmdList, const Viewport& viewport ) const
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      if( viewport.width != 0 && viewport.height != 0 )
      {
         cmdBuffer->setViewport( viewport );
      }
      else
      {
         // This is a null viewport, give a default one instead
         const VkExtent2D& swapchainExtent = m_mainSwapchain->getVKExtent();

         const Viewport defaultViewport =
             Viewport( 0.0f, 0.0f, swapchainExtent.width, swapchainExtent.height );

         cmdBuffer->setViewport( defaultViewport );
      }
   }

   void setScissor( CmdListHandle cmdList, const Rectangle& scissor ) const
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      if( scissor.extent.width > 0 && scissor.extent.height > 0 )
      {
         cmdBuffer->setScissor( scissor );
      }
      else
      {
         // This is a null scissor, give a default one instead
         const VkExtent2D& swapchainExtent = m_mainSwapchain->getVKExtent();

         const Rectangle defaultScissor =
             Rectangle( { 0.0f, 0.0f }, { swapchainExtent.width, swapchainExtent.height } );

         cmdBuffer->setScissor( defaultScissor );
      }
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

   void bindIndexBuffer(
       CmdListHandle cmdList,
       IndexBufferHandle bufferHandle,
       IndexType type,
       uint32_t offset ) const
   {
      if( !bufferHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid index buffer" );
         return;
      }

      const auto cmdBuffer   = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto indexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindIndexBuffer( indexBuffer, type, offset );
   }

   void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       uint32_t binding,
       uint32_t set ) const
   {
      if( !texHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindTexture( texture, binding, set );
   }

   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t binding, uint32_t set )
       const
   {
      if( !texHandle )
      {
         CYDASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::GENERAL );
      cmdBuffer->bindImage( texture, binding, set );
   }

   void bindBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t binding,
       uint32_t set,
       uint32_t offset,
       uint32_t range ) const
   {
      auto cmdBuffer    = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindBuffer( buffer, binding, set, offset, range );
   }

   void bindUniformBuffer(
       CmdListHandle cmdList,
       BufferHandle bufferHandle,
       uint32_t binding,
       uint32_t set,
       uint32_t offset,
       uint32_t range ) const
   {
      auto cmdBuffer           = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto uniformBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      cmdBuffer->bindUniformBuffer( uniformBuffer, binding, set, offset, range );
   }

   void updateConstantBuffer(
       CmdListHandle cmdList,
       PipelineStageFlag stages,
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

   TextureHandle createTexture( const TextureDescription& desc )
   {
      // Creating GPU texture
      vk::Texture* texture = m_mainDevice.createTexture( desc );

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   TextureHandle createTexture(
       CmdListHandle transferList,
       const TextureDescription& desc,
       uint32_t layerCount,
       const void* const* ppTexels )
   {
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( desc.size );

      const size_t layerSize = desc.size / desc.layers;
      for( uint32_t i = 0; i < layerCount; ++i )
      {
         staging->copy( ppTexels[i], i * layerSize, layerSize );
      }

      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice.createTexture( desc );

      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::TRANSFER_DST );

      cmdBuffer->uploadBufferToTex( staging, texture );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::SHADER_READ );

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   TextureHandle
   createTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels )
   {
      // Staging
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( desc.size );
      staging->copy( pTexels, 0, desc.size );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice.createTexture( desc );

      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::TRANSFER_DST );

      cmdBuffer->uploadBufferToTex( staging, texture );

      vk::Barriers::ImageMemory( cmdBuffer, texture, CYD::ImageLayout::SHADER_READ );

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   VertexBufferHandle createVertexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       uint32_t stride,
       const void* pVertices,
       const std::string_view name )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      const size_t bufferSize = static_cast<size_t>( count ) * stride;

      // Staging
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( bufferSize );
      staging->copy( pVertices, 0, bufferSize );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* vertexBuffer = m_mainDevice.createVertexBuffer( bufferSize, name );
      cmdBuffer->copyBuffer( staging, vertexBuffer );

      return m_coreHandles.add( vertexBuffer, HandleType::VERTEXBUFFER );
   }

   IndexBufferHandle createIndexBuffer(
       CmdListHandle transferList,
       uint32_t count,
       const void* pIndices,
       const std::string_view name )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( transferList ) );

      // TODO Dynamic uint32_t/uint16_t
      const size_t bufferSize = count * sizeof( uint32_t );

      // Staging
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( bufferSize );
      staging->copy( pIndices, 0, bufferSize );
      m_cmdListDeps[transferList].emplace_back( staging );

      // Uploading to GPU
      vk::Buffer* indexBuffer = m_mainDevice.createIndexBuffer( bufferSize, name );
      cmdBuffer->copyBuffer( staging, indexBuffer );

      return m_coreHandles.add( indexBuffer, HandleType::INDEXBUFFER );
   }

   BufferHandle createUniformBuffer( size_t size, const std::string_view name )
   {
      auto uniformBuffer = m_mainDevice.createUniformBuffer( size, name );

      return m_coreHandles.add( uniformBuffer, HandleType::BUFFER );
   }

   BufferHandle createBuffer( size_t size, const std::string_view name )
   {
      auto deviceBuffer = m_mainDevice.createBuffer( size, name );

      return m_coreHandles.add( deviceBuffer, HandleType::BUFFER );
   }

   void* addDebugTexture( TextureHandle textureHandle )
   {
      if( textureHandle )
      {
         vk::SamplerCache& samplerCache = m_mainDevice.getSamplerCache();
         VkSampler vkSampler            = samplerCache.findOrCreate( {} );

         auto texture = static_cast<vk::Texture*>( m_coreHandles.get( textureHandle ) );
         return ImGui_ImplVulkan_AddTexture(
             vkSampler,
             texture->getVKImageView(),
             vk::TypeConversions::cydToVkImageLayout( texture->getLayout() ) );
      }

      return nullptr;
   }

   void removeDebugTexture( void* texture )
   {
      ImGui_ImplVulkan_RemoveTexture( static_cast<VkDescriptorSet>( texture ) );
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
         auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );
         texture->decUse();

         m_coreHandles.remove( texHandle );
      }
   }

   void destroyVertexBuffer( VertexBufferHandle bufferHandle )
   {
      if( bufferHandle )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->decUse();

         m_coreHandles.remove( bufferHandle );
      }
   }

   void destroyIndexBuffer( IndexBufferHandle bufferHandle )
   {
      if( bufferHandle )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->decUse();

         m_coreHandles.remove( bufferHandle );
      }
   }

   void destroyBuffer( BufferHandle bufferHandle )
   {
      if( bufferHandle )
      {
         auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
         buffer->decUse();

         m_coreHandles.remove( bufferHandle );
      }
   }

   void prepareFrame() const
   {
      if( m_hasUI )
      {
         ImGui_ImplVulkan_NewFrame();
         ImGui_ImplGlfw_NewFrame();
         ImGui::NewFrame();
      }

      m_mainSwapchain->acquireImage();
   }

   void beginRendering( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->beginRendering( *m_mainSwapchain );
   }

   void beginRendering(
       CmdListHandle cmdList,
       const FramebufferInfo& targetsInfo,
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

      vk::Barriers::Pipeline(
          cmdBuffer, CYD::PipelineStage::COMPUTE_STAGE, CYD::PipelineStage::COMPUTE_STAGE );

      cmdBuffer->dispatch( workX, workY, workZ );
   }

   void presentFrame() const
   {
      if( m_hasUI )
      {
         ImGui::EndFrame();
      }

      m_mainSwapchain->present();
   }

   void
   beginDebugRange( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      vk::DebugUtilsLabel::Begin( *cmdBuffer, name, color );
   }

   void endDebugRange( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      vk::DebugUtilsLabel::End( *cmdBuffer );
   }

   void
   insertDebugLabel( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      vk::DebugUtilsLabel::Insert( *cmdBuffer, name, color );
   }

  private:
   const Window& m_window;
   vk::Instance m_instance;
   vk::Surface m_surface;
   vk::DeviceManager m_devices;

   vk::Device& m_mainDevice;
   vk::Swapchain* m_mainSwapchain;

   HandleManager m_coreHandles;

   // Used to store intermediate buffers like staging buffers to be able to flag them as unused once
   // the command list is getting destroyed
   using CmdListDependencyMap = std::unordered_map<CmdListHandle, std::vector<vk::Buffer*>>;
   CmdListDependencyMap m_cmdListDeps;

   bool m_hasUI = false;
};

// =================================================================================================
// Link
VKRenderBackend::VKRenderBackend( const Window& window ) : _imp( new VKRenderBackendImp( window ) )
{
}

VKRenderBackend::~VKRenderBackend() { delete _imp; }

bool VKRenderBackend::initializeUI() { return _imp->initializeUI(); }
void VKRenderBackend::uninitializeUI() { _imp->uninitializeUI(); }
void VKRenderBackend::drawUI( CmdListHandle cmdList ) { _imp->drawUI( cmdList ); }
void VKRenderBackend::cleanup() { _imp->cleanup(); }
void VKRenderBackend::waitUntilIdle() { _imp->waitUntilIdle(); }

CmdListHandle VKRenderBackend::createCommandList(
    QueueUsageFlag usage,
    const std::string_view name,
    bool presentable )
{
   return _imp->createCommandList( usage, name, presentable );
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

void VKRenderBackend::syncOnCommandList( CmdListHandle from, CmdListHandle to )
{
   _imp->syncOnCommandList( from, to );
}

void VKRenderBackend::destroyCommandList( CmdListHandle cmdList )
{
   _imp->destroyCommandList( cmdList );
}

void VKRenderBackend::syncOnSwapchain( CmdListHandle cmdList ) { _imp->syncOnSwapchain( cmdList ); }

void VKRenderBackend::syncToSwapchain( CmdListHandle cmdList ) { _imp->syncToSwapchain( cmdList ); }

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
    IndexType type,
    uint32_t offset )
{
   _imp->bindIndexBuffer( cmdList, bufferHandle, type, offset );
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
    uint32_t binding,
    uint32_t offset,
    uint32_t range )
{
   _imp->bindBuffer( cmdList, bufferHandle, set, binding, offset, range );
}

void VKRenderBackend::bindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t set,
    uint32_t binding,
    uint32_t offset,
    uint32_t range )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle, set, binding, offset, range );
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
    PipelineStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData )
{
   _imp->updateConstantBuffer( cmdList, stages, offset, size, pData );
}

TextureHandle VKRenderBackend::createTexture( const CYD::TextureDescription& desc )
{
   return _imp->createTexture( desc );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const CYD::TextureDescription& desc,
    const void* pTexels )
{
   return _imp->createTexture( transferList, desc, pTexels );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle transferList,
    const CYD::TextureDescription& desc,
    uint32_t layerCount,
    const void* const* ppTexels )
{
   return _imp->createTexture( transferList, desc, layerCount, ppTexels );
}

VertexBufferHandle VKRenderBackend::createVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices,
    const std::string_view name )
{
   return _imp->createVertexBuffer( transferList, count, stride, pVertices, name );
}

IndexBufferHandle VKRenderBackend::createIndexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    const void* pIndices,
    const std::string_view name )
{
   return _imp->createIndexBuffer( transferList, count, pIndices, name );
}

BufferHandle VKRenderBackend::createUniformBuffer( size_t size, const std::string_view name )
{
   return _imp->createUniformBuffer( size, name );
}

BufferHandle VKRenderBackend::createBuffer( size_t size, const std::string_view name )
{
   return _imp->createBuffer( size, name );
}

void* VKRenderBackend::addDebugTexture( TextureHandle textureHandle )
{
   return _imp->addDebugTexture( textureHandle );
}

void VKRenderBackend::removeDebugTexture( void* texture )
{
   return _imp->removeDebugTexture( texture );
}

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

void VKRenderBackend::beginRendering( CmdListHandle cmdList ) { _imp->beginRendering( cmdList ); }

void VKRenderBackend::beginRendering(
    CmdListHandle cmdList,
    const FramebufferInfo& targetsInfo,
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

void VKRenderBackend::beginDebugRange(
    CmdListHandle cmdList,
    const char* name,
    const std::array<float, 4>& color )
{
   _imp->beginDebugRange( cmdList, name, color );
}

void VKRenderBackend::endDebugRange( CmdListHandle cmdList ) { _imp->endDebugRange( cmdList ); }

void VKRenderBackend::insertDebugLabel(
    CmdListHandle cmdList,
    const char* name,
    const std::array<float, 4>& color )
{
   _imp->insertDebugLabel( cmdList, name, color );
}
}
