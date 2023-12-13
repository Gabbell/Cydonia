#include <Graphics/GRIS/Backends/VKRenderBackend.h>

#include <Common/Assert.h>

#include <Input/GLFWWindow.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/PipelineInfos.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/VertexList.h>
#include <Graphics/Handles/ResourceHandleManager.h>
#include <Graphics/Utility/GraphicsIO.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/DebugUtilsLabel.h>
#include <Graphics/Vulkan/DescriptorPool.h>
#include <Graphics/Vulkan/Synchronization.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/DeviceManager.h>
#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/SamplerCache.h>
#include <Graphics/Vulkan/RenderPassCache.h>
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
      const uint32_t imageCount = vk::Swapchain::MAX_FRAMES_IN_FLIGHT;

      // Initializing swapchain on main device
      m_scInfo.imageCount = imageCount;
      m_scInfo.extent     = window.getExtent();
      m_scInfo.format     = PixelFormat::RGBA8_SRGB;
      m_scInfo.space      = ColorSpace::SRGB_NONLINEAR;
      m_scInfo.mode       = PresentMode::FIFO;

      m_mainSwapchain = m_mainDevice.createSwapchain( m_scInfo );

      CYD_ASSERT( m_mainSwapchain );

      vk::DebugUtilsLabel::Initialize( m_mainDevice );
   }

   ~VKRenderBackendImp()
   {
      //
   }

   bool initializeUIBackend()
   {
      // Initialize ImGui
      ImGui_ImplGlfw_InitForVulkan( m_window.getGLFWwindow(), true );

      const uint32_t graphicsFamilyIndex = m_mainDevice.getQueueFamilyIndexFromUsage( GRAPHICS );

      const vk::Device::QueueFamily& queueFamily =
          m_mainDevice.getQueueFamilyFromIndex( graphicsFamilyIndex );

      if( queueFamily.queues.empty() )
      {
         CYD_ASSERT( !"Could not find a graphics queue to initialize UI" );
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

      vk::RenderPassInfo renderPass    = m_mainSwapchain->getRenderPass();
      renderPass.attachments[0].format = CYD::PixelFormat::RGBA32F;

      ImGui_ImplVulkan_Init(
          &init_info, m_mainDevice.getRenderPassCache().findOrCreate( renderPass ) );

      // Loading fonts
      const CmdListHandle cmdList =
          createCommandList( GRAPHICS, "ImGui Font Loading", false, false );
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      ImGui_ImplVulkan_CreateFontsTexture( cmdBuffer->getVKCmdBuffer() );

      submitCommandList( cmdList );
      waitOnCommandList( cmdList );
      destroyCommandList( cmdList );

      ImGui_ImplVulkan_DestroyFontUploadObjects();

      ImVec4* colors                         = ImGui::GetStyle().Colors;
      colors[ImGuiCol_Text]                  = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
      colors[ImGuiCol_TextDisabled]          = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
      colors[ImGuiCol_WindowBg]              = ImVec4( 0.06f, 0.06f, 0.06f, 0.94f );
      colors[ImGuiCol_ChildBg]               = ImVec4( 1.00f, 1.00f, 1.00f, 0.00f );
      colors[ImGuiCol_PopupBg]               = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
      colors[ImGuiCol_Border]                = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
      colors[ImGuiCol_BorderShadow]          = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
      colors[ImGuiCol_FrameBg]               = ImVec4( 0.20f, 0.21f, 0.22f, 0.54f );
      colors[ImGuiCol_FrameBgHovered]        = ImVec4( 0.40f, 0.40f, 0.40f, 0.40f );
      colors[ImGuiCol_FrameBgActive]         = ImVec4( 0.18f, 0.18f, 0.18f, 0.67f );
      colors[ImGuiCol_TitleBg]               = ImVec4( 0.04f, 0.04f, 0.04f, 1.00f );
      colors[ImGuiCol_TitleBgActive]         = ImVec4( 0.29f, 0.29f, 0.29f, 1.00f );
      colors[ImGuiCol_TitleBgCollapsed]      = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
      colors[ImGuiCol_MenuBarBg]             = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
      colors[ImGuiCol_ScrollbarBg]           = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
      colors[ImGuiCol_ScrollbarGrab]         = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
      colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
      colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
      colors[ImGuiCol_CheckMark]             = ImVec4( 0.94f, 0.94f, 0.94f, 1.00f );
      colors[ImGuiCol_SliderGrab]            = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
      colors[ImGuiCol_SliderGrabActive]      = ImVec4( 0.86f, 0.86f, 0.86f, 1.00f );
      colors[ImGuiCol_Button]                = ImVec4( 0.44f, 0.44f, 0.44f, 0.40f );
      colors[ImGuiCol_ButtonHovered]         = ImVec4( 0.46f, 0.47f, 0.48f, 1.00f );
      colors[ImGuiCol_ButtonActive]          = ImVec4( 0.42f, 0.42f, 0.42f, 1.00f );
      colors[ImGuiCol_Header]                = ImVec4( 0.70f, 0.70f, 0.70f, 0.31f );
      colors[ImGuiCol_HeaderHovered]         = ImVec4( 0.70f, 0.70f, 0.70f, 0.80f );
      colors[ImGuiCol_HeaderActive]          = ImVec4( 0.48f, 0.50f, 0.52f, 1.00f );
      colors[ImGuiCol_Separator]             = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
      colors[ImGuiCol_SeparatorHovered]      = ImVec4( 0.72f, 0.72f, 0.72f, 0.78f );
      colors[ImGuiCol_SeparatorActive]       = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
      colors[ImGuiCol_ResizeGrip]            = ImVec4( 0.91f, 0.91f, 0.91f, 0.25f );
      colors[ImGuiCol_ResizeGripHovered]     = ImVec4( 0.81f, 0.81f, 0.81f, 0.67f );
      colors[ImGuiCol_ResizeGripActive]      = ImVec4( 0.46f, 0.46f, 0.46f, 0.95f );
      colors[ImGuiCol_PlotLines]             = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
      colors[ImGuiCol_PlotLinesHovered]      = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
      colors[ImGuiCol_PlotHistogram]         = ImVec4( 0.73f, 0.60f, 0.15f, 1.00f );
      colors[ImGuiCol_PlotHistogramHovered]  = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
      colors[ImGuiCol_TextSelectedBg]        = ImVec4( 0.87f, 0.87f, 0.87f, 0.35f );
      colors[ImGuiCol_DragDropTarget]        = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
      colors[ImGuiCol_NavHighlight]          = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
      colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );

      m_hasUI = true;

      return true;
   }

   void uninitializeUIBackend()
   {
      if( m_hasUI )
      {
         ImGui_ImplVulkan_Shutdown();
         ImGui_ImplGlfw_Shutdown();
      }

      m_hasUI = false;
   }

   void drawUI( CmdListHandle cmdList ) const
   {
      ImGui::Render();

      ImDrawData* drawData = ImGui::GetDrawData();

      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      ImGui_ImplVulkan_RenderDrawData( drawData, cmdBuffer->getVKCmdBuffer() );
   }

   void cleanup() const { m_mainDevice.cleanup(); }
   void reloadShaders() { m_reloadShadersQueued = true; }

   void waitUntilIdle() const { m_mainDevice.waitUntilIdle(); }

   CmdListHandle createCommandList(
       QueueUsageFlag usage,
       const std::string_view name,
       bool async,
       bool presentable )
   {
      const auto cmdBuffer = m_mainDevice.createCommandBuffer( usage, name, async, presentable );
      cmdBuffer->startRecording();
      return m_coreHandles.add( cmdBuffer, HandleType::CMDLIST );
   }

   void submitCommandList( CmdListHandle cmdList ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->endRecording();
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
         CYD_ASSERT( !"VKRenderBackend: Tried to bind an invalid vertex buffer" );
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
         CYD_ASSERT( !"VKRenderBackend: Tried to bind an invalid index buffer" );
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
         CYD_ASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindTexture( texture, binding, set );
   }

   void bindTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       const SamplerInfo& sampler,
       uint32_t binding,
       uint32_t set ) const
   {
      if( !texHandle )
      {
         CYD_ASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->bindTexture( texture, sampler, binding, set );
   }

   void bindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t binding, uint32_t set )
       const
   {
      if( !texHandle )
      {
         CYD_ASSERT( !"VKRenderBackend: Tried to bind an invalid texture" );
         return;
      }

      auto cmdBuffer     = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

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

   TextureHandle
   createTexture( CmdListHandle cmdList, const TextureDescription& desc, const void* pTexels )
   {
      const size_t size = desc.width * desc.height * GetPixelSizeInBytes( desc.format );

      // Staging
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( size );

      const UploadToBufferInfo uploadInfo = { 0, size };
      staging->copy( pTexels, uploadInfo );

      m_cmdListDeps[cmdList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice.createTexture( desc );

      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      BufferToTextureInfo copyInfo;
      copyInfo.srcOffset = 0;
      copyInfo.dstOffset = Offset2D( 0, 0 );
      copyInfo.dstExtent = Extent2D( desc.width, desc.height );
      cmdBuffer->copyBufferToTexture( staging, texture, copyInfo );

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   TextureHandle createTexture(
       CmdListHandle cmdList,
       const TextureDescription& desc,
       uint32_t layerCount,
       const void* const* ppTexels )
   {
      const size_t size = desc.width * desc.height * GetPixelSizeInBytes( desc.format );

      vk::Buffer* staging = m_mainDevice.createStagingBuffer( size );

      const size_t layerSize = size / desc.depth;
      for( uint32_t i = 0; i < layerCount; ++i )
      {
         const UploadToBufferInfo uploadInfo = { i * layerSize, layerSize };
         staging->copy( ppTexels[i], uploadInfo );
      }

      m_cmdListDeps[cmdList].emplace_back( staging );

      // Uploading to GPU
      vk::Texture* texture = m_mainDevice.createTexture( desc );

      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      BufferToTextureInfo copyInfo;
      copyInfo.srcOffset = 0;
      copyInfo.dstOffset = Offset2D( 0, 0 );
      copyInfo.dstExtent = Extent2D( desc.width, desc.height );
      cmdBuffer->copyBufferToTexture( staging, texture, copyInfo );

      return m_coreHandles.add( texture, HandleType::TEXTURE );
   }

   void generateMipmaps( CmdListHandle cmdList, TextureHandle texHandle )
   {
      const auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto texture   = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->generateMipmaps( texture );
   }

   VertexBufferHandle createVertexBuffer( size_t size, const std::string_view name )
   {
      vk::Buffer* vertexBuffer = m_mainDevice.createVertexBuffer( size, name );
      return m_coreHandles.add( vertexBuffer, HandleType::VERTEXBUFFER );
   }

   IndexBufferHandle createIndexBuffer( size_t size, const std::string_view name )
   {
      vk::Buffer* indexBuffer = m_mainDevice.createIndexBuffer( size, name );
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

   void* addDebugTexture( TextureHandle texture, uint32_t layer )
   {
      auto vkTexture = static_cast<vk::Texture*>( m_coreHandles.get( texture ) );
      if( vkTexture == nullptr ) return nullptr;

      vk::SamplerCache& samplerCache = m_mainDevice.getSamplerCache();
      VkSampler vkSampler            = samplerCache.findOrCreate( {} );

      return ImGui_ImplVulkan_AddTexture(
          vkSampler,
          vkTexture->getIndexedVKImageView( layer, 0 ),
          vk::Synchronization::GetLayoutFromAccess( vkTexture->getPreviousAccess() ) );
   }

   void updateDebugTexture( CmdListHandle cmdList, TextureHandle texture )
   {
      if( texture )
      {
         auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
         auto vkTexture = static_cast<vk::Texture*>( m_coreHandles.get( texture ) );

         vk::Synchronization::ImageMemory( cmdBuffer, vkTexture, Access::FRAGMENT_SHADER_READ );
      }
   }

   void removeDebugTexture( void* texture )
   {
      ImGui_ImplVulkan_RemoveTexture( static_cast<VkDescriptorSet>( texture ) );
   }

   void uploadToBuffer(
       BufferHandle bufferHandle,
       const void* pData,
       const UploadToBufferInfo& info ) const
   {
      CYD_ASSERT( bufferHandle );

      auto buffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );
      buffer->copy( pData, info );
   }

   void uploadToVertexBuffer(
       CmdListHandle cmdList,
       VertexBufferHandle bufferHandle,
       const VertexList& vertices )
   {
      CYD_ASSERT( bufferHandle );

      const auto cmdBuffer    = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto vertexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      const size_t copySize = vertices.getSize();

      CYD_ASSERT( copySize <= vertexBuffer->getSize() );

      // Staging
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( copySize );

      const UploadToBufferInfo uploadInfo = { 0, copySize };
      staging->copy( vertices.getData(), uploadInfo );

      m_cmdListDeps[cmdList].emplace_back( staging );

      // Uploading to GPU
      const BufferCopyInfo copyInfo = { 0, 0, copySize };
      cmdBuffer->copyBuffer( staging, vertexBuffer, copyInfo );
   }

   void uploadToIndexBuffer(
       CmdListHandle cmdList,
       IndexBufferHandle bufferHandle,
       const void* pIndices,
       const UploadToBufferInfo& info )
   {
      const auto cmdBuffer   = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      const auto indexBuffer = static_cast<vk::Buffer*>( m_coreHandles.get( bufferHandle ) );

      // TODO IndexList
      // Staging
      vk::Buffer* staging = m_mainDevice.createStagingBuffer( info.size );

      staging->copy( pIndices, info );

      m_cmdListDeps[cmdList].emplace_back( staging );

      const BufferCopyInfo copyInfo = { 0, 0, info.size };
      cmdBuffer->copyBuffer( staging, indexBuffer, copyInfo );
   }

   void copyTexture(
       CmdListHandle cmdList,
       TextureHandle srcTexHandle,
       TextureHandle dstTexHandle,
       const TextureCopyInfo& info ) const
   {
      CYD_ASSERT( cmdList && srcTexHandle && dstTexHandle );

      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto src       = static_cast<vk::Texture*>( m_coreHandles.get( srcTexHandle ) );
      auto dst       = static_cast<vk::Texture*>( m_coreHandles.get( dstTexHandle ) );

      cmdBuffer->copyTexture( src, dst, info );
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

   void beginFrame()
   {
      m_window.poll();

      if( m_hasUI )
      {
         ImGui_ImplVulkan_NewFrame();
         ImGui_ImplGlfw_NewFrame();
         ImGui::NewFrame();
      }

      // CPU wait for all work to be done for this frame before acquiring next
      // image Otherwise, there is a possibility that we feed the GPU too much
      // and cap our CPU-side resource limits
      // m_mainDevice.waitOnFrame( currentFrame );

      // Keeping swapchain the same size as the window extent
      const Extent2D& windowExtent      = m_window.getExtent();
      const VkExtent2D& swapchainExtent = m_mainSwapchain->getVKExtent();
      if( !m_mainSwapchain->acquireImage() || windowExtent.width != swapchainExtent.width ||
          windowExtent.height != swapchainExtent.height )
      {
         waitUntilIdle();

         m_scInfo.extent = windowExtent;
         m_mainSwapchain->recreateSwapchain( m_scInfo );

         const bool acquiredSuccess = m_mainSwapchain->acquireImage();
         CYD_ASSERT( acquiredSuccess );
      }

      m_mainSwapchain->setClear( true );
   }

   void beginRendering( CmdListHandle cmdList )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      cmdBuffer->beginRendering( *m_mainSwapchain );

      m_mainSwapchain->setClear( false );
   }

   void beginRendering( CmdListHandle cmdList, const Framebuffer& fb, uint32_t layer ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      // Fetching textures
      const Framebuffer::RenderTargets& renderTargets = fb.getRenderTargets();
      std::vector<vk::Texture*> vkTextures;
      vkTextures.reserve( renderTargets.size() );
      for( const auto& renderTarget : renderTargets )
      {
         const auto vkTexture =
             static_cast<vk::Texture*>( m_coreHandles.get( renderTarget.texture ) );
         if( vkTexture )
         {
            vkTextures.push_back( vkTexture );
         }
      }

      cmdBuffer->beginRendering( fb, vkTextures, layer );
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

   void draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->draw( vertexCount, 1, firstVertex, 0 );
   }

   void drawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->drawIndexed( indexCount, 1, firstIndex, 0 );
   }

   void drawInstanced(
       CmdListHandle cmdList,
       size_t vertexCount,
       size_t instanceCount,
       size_t firstVertex,
       size_t firstInstance )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->draw( vertexCount, instanceCount, firstVertex, firstInstance );
   }

   void drawIndexedInstanced(
       CmdListHandle cmdList,
       size_t indexCount,
       size_t instanceCount,
       size_t firstIndex,
       size_t firstInstance )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      cmdBuffer->drawIndexed( indexCount, instanceCount, firstIndex, firstInstance );
   }

   void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );

      cmdBuffer->dispatch( workX, workY, workZ );
   }

   void clearTexture(
       CmdListHandle cmdList,
       TextureHandle texHandle,
       const ClearValue& clearVal ) const
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto texture   = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->clearTexture( texture, clearVal );
   }

   void copyToSwapchain( CmdListHandle cmdList, TextureHandle texHandle )
   {
      auto cmdBuffer = static_cast<vk::CommandBuffer*>( m_coreHandles.get( cmdList ) );
      auto texture   = static_cast<vk::Texture*>( m_coreHandles.get( texHandle ) );

      cmdBuffer->copyToSwapchain( texture, *m_mainSwapchain );
   }

   void presentFrame()
   {
      if( m_hasUI )
      {
         ImGui::EndFrame();
      }

      m_mainSwapchain->present();

      if( m_reloadShadersQueued )
      {
         m_mainDevice.waitUntilIdle();
         m_mainDevice.clearPipelines();
         m_reloadShadersQueued = false;
      }
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

   CYD::SwapchainInfo m_scInfo;
   vk::Swapchain* m_mainSwapchain;

   bool m_reloadShadersQueued = false;

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

bool VKRenderBackend::initializeUIBackend() { return _imp->initializeUIBackend(); }
void VKRenderBackend::uninitializeUIBackend() { _imp->uninitializeUIBackend(); }
void VKRenderBackend::drawUI( CmdListHandle cmdList ) { _imp->drawUI( cmdList ); }
void VKRenderBackend::cleanup() { _imp->cleanup(); }
void VKRenderBackend::reloadShaders() { _imp->reloadShaders(); }
void VKRenderBackend::waitUntilIdle() { _imp->waitUntilIdle(); }

CmdListHandle VKRenderBackend::createCommandList(
    QueueUsageFlag usage,
    const std::string_view name,
    bool async,
    bool presentable )
{
   return _imp->createCommandList( usage, name, async, presentable );
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
    uint32_t binding,
    uint32_t set )
{
   _imp->bindTexture( cmdList, texHandle, binding, set );
}

void VKRenderBackend::bindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const SamplerInfo& sampler,
    uint32_t binding,
    uint32_t set )
{
   _imp->bindTexture( cmdList, texHandle, sampler, binding, set );
}

void VKRenderBackend::bindImage(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    uint32_t binding,
    uint32_t set )
{
   _imp->bindImage( cmdList, texHandle, binding, set );
}

void VKRenderBackend::bindBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set,
    uint32_t offset,
    uint32_t range )
{
   _imp->bindBuffer( cmdList, bufferHandle, binding, set, offset, range );
}

void VKRenderBackend::bindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set,
    uint32_t offset,
    uint32_t range )
{
   _imp->bindUniformBuffer( cmdList, bufferHandle, binding, set, offset, range );
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
    CmdListHandle cmdList,
    const CYD::TextureDescription& desc,
    const void* pTexels )
{
   return _imp->createTexture( cmdList, desc, pTexels );
}

TextureHandle VKRenderBackend::createTexture(
    CmdListHandle cmdList,
    const CYD::TextureDescription& desc,
    uint32_t layerCount,
    const void* const* ppTexels )
{
   return _imp->createTexture( cmdList, desc, layerCount, ppTexels );
}

void VKRenderBackend::generateMipmaps( CmdListHandle cmdList, TextureHandle texHandle )
{
   return _imp->generateMipmaps( cmdList, texHandle );
}

VertexBufferHandle VKRenderBackend::createVertexBuffer( size_t size, const std::string_view name )
{
   return _imp->createVertexBuffer( size, name );
}

IndexBufferHandle VKRenderBackend::createIndexBuffer( size_t size, const std::string_view name )
{
   return _imp->createIndexBuffer( size, name );
}

BufferHandle VKRenderBackend::createUniformBuffer( size_t size, const std::string_view name )
{
   return _imp->createUniformBuffer( size, name );
}

BufferHandle VKRenderBackend::createBuffer( size_t size, const std::string_view name )
{
   return _imp->createBuffer( size, name );
}

void* VKRenderBackend::addDebugTexture( TextureHandle textureHandle, uint32_t layer )
{
   return _imp->addDebugTexture( textureHandle, layer );
}

void VKRenderBackend::updateDebugTexture( CmdListHandle cmdList, TextureHandle textureHandle )
{
   _imp->updateDebugTexture( cmdList, textureHandle );
}

void VKRenderBackend::removeDebugTexture( void* texture ) { _imp->removeDebugTexture( texture ); }

void VKRenderBackend::uploadToBuffer(
    BufferHandle bufferHandle,
    const void* pData,
    const UploadToBufferInfo& info )
{
   _imp->uploadToBuffer( bufferHandle, pData, info );
}

void VKRenderBackend::uploadToVertexBuffer(
    CmdListHandle cmdList,
    VertexBufferHandle bufferHandle,
    const VertexList& vertices )
{
   _imp->uploadToVertexBuffer( cmdList, bufferHandle, vertices );
}

void VKRenderBackend::uploadToIndexBuffer(
    CmdListHandle cmdList,
    IndexBufferHandle bufferHandle,
    const void* pIndices,
    const UploadToBufferInfo& info )
{
   _imp->uploadToIndexBuffer( cmdList, bufferHandle, pIndices, info );
}

void VKRenderBackend::copyTexture(
    CmdListHandle cmdList,
    TextureHandle srcTexHandle,
    TextureHandle dstTexHandle,
    const TextureCopyInfo& info )
{
   _imp->copyTexture( cmdList, srcTexHandle, dstTexHandle, info );
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

void VKRenderBackend::beginFrame() { _imp->beginFrame(); }

void VKRenderBackend::beginRendering( CmdListHandle cmdList ) { _imp->beginRendering( cmdList ); }

void VKRenderBackend::beginRendering( CmdListHandle cmdList, const Framebuffer& fb, uint32_t layer )
{
   _imp->beginRendering( cmdList, fb, layer );
}

void VKRenderBackend::nextPass( CmdListHandle cmdList ) { _imp->nextPass( cmdList ); }

void VKRenderBackend::endRendering( CmdListHandle cmdList ) { _imp->endRendering( cmdList ); }

void VKRenderBackend::draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex )
{
   _imp->draw( cmdList, vertexCount, firstVertex );
}

void VKRenderBackend::drawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex )
{
   _imp->drawIndexed( cmdList, indexCount, firstIndex );
}

void VKRenderBackend::drawInstanced(
    CmdListHandle cmdList,
    size_t vertexCount,
    size_t instanceCount,
    size_t firstVertex,
    size_t firstInstance )
{
   _imp->drawInstanced( cmdList, vertexCount, instanceCount, firstVertex, firstInstance );
}

void VKRenderBackend::drawIndexedInstanced(
    CmdListHandle cmdList,
    size_t indexCount,
    size_t instanceCount,
    size_t firstIndex,
    size_t firstInstance )
{
   _imp->drawIndexedInstanced( cmdList, indexCount, instanceCount, firstIndex, firstInstance );
}

void VKRenderBackend::dispatch(
    CmdListHandle cmdList,
    uint32_t workX,
    uint32_t workY,
    uint32_t workZ )
{
   _imp->dispatch( cmdList, workX, workY, workZ );
}

void VKRenderBackend::clearTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const ClearValue& clearVal )
{
   _imp->clearTexture( cmdList, texHandle, clearVal );
}

void VKRenderBackend::copyToSwapchain( CmdListHandle cmdList, TextureHandle texHandle )
{
   _imp->copyToSwapchain( cmdList, texHandle );
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
