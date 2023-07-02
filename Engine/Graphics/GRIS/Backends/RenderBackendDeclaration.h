#pragma once

#define GRIS_RENDERBACKEND_DECLARATION( NAME )                                                     \
                                                                                                   \
   namespace CYD                                                                                   \
   {                                                                                               \
   class Window;                                                                                   \
   class NAMEImp;                                                                                  \
   }                                                                                               \
                                                                                                   \
   namespace CYD                                                                                   \
   {                                                                                               \
   class NAME final : public RenderBackend                                                         \
   {                                                                                               \
     public:                                                                                       \
      NAME( const Window& window );                                                                \
      NON_COPIABLE( NAME );                                                                        \
      virtual ~NAME();                                                                             \
                                                                                                   \
      bool initializeUIBackend() override;                                                                \
      void uninitializeUIBackend() override;                                                              \
      void drawUI( CmdListHandle cmdList ) override;                                               \
                                                                                                   \
      void cleanup() override;                                                                     \
                                                                                                   \
      void waitUntilIdle() override;                                                               \
                                                                                                   \
      CmdListHandle createCommandList(                                                             \
          QueueUsageFlag usage,                                                                    \
          const std::string_view name,                                                             \
          bool presentable ) override;                                                             \
                                                                                                   \
      void startRecordingCommandList( CmdListHandle cmdList ) override;                            \
      void endRecordingCommandList( CmdListHandle cmdList ) override;                              \
      void submitCommandList( CmdListHandle cmdList ) override;                                    \
      void resetCommandList( CmdListHandle cmdList ) override;                                     \
      void waitOnCommandList( CmdListHandle cmdList ) override;                                    \
      void syncOnCommandList( CmdListHandle from, CmdListHandle to ) override;                     \
      void destroyCommandList( CmdListHandle cmdList ) override;                                   \
                                                                                                   \
      void syncOnSwapchain( CmdListHandle cmdList ) override;                                      \
      void syncToSwapchain( CmdListHandle cmdList ) override;                                      \
                                                                                                   \
      void setViewport( CmdListHandle cmdList, const Viewport& viewport ) override;                \
      void setScissor( CmdListHandle cmdList, const Rectangle& scissor ) override;                 \
                                                                                                   \
      void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ) override;    \
      void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo ) override;     \
                                                                                                   \
      void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ) override;    \
      void bindIndexBuffer(                                                                        \
          CmdListHandle cmdList,                                                                   \
          IndexBufferHandle bufferHandle,                                                          \
          IndexType type,                                                                          \
          uint32_t offset ) override;                                                              \
                                                                                                   \
      void bindTexture(                                                                            \
          CmdListHandle cmdList,                                                                   \
          TextureHandle texHandle,                                                                 \
          uint32_t set,                                                                            \
          uint32_t binding ) override;                                                             \
      void bindImage(                                                                              \
          CmdListHandle cmdList,                                                                   \
          TextureHandle texHandle,                                                                 \
          uint32_t set,                                                                            \
          uint32_t binding ) override;                                                             \
      void bindBuffer(                                                                             \
          CmdListHandle cmdList,                                                                   \
          BufferHandle bufferHandle,                                                               \
          uint32_t binding,                                                                        \
          uint32_t set,                                                                            \
          uint32_t offset,                                                                         \
          uint32_t range ) override;                                                               \
      void bindUniformBuffer(                                                                      \
          CmdListHandle cmdList,                                                                   \
          BufferHandle bufferHandle,                                                               \
          uint32_t binding,                                                                        \
          uint32_t set,                                                                            \
          uint32_t offset,                                                                         \
          uint32_t range ) override;                                                               \
                                                                                                   \
      void updateConstantBuffer(                                                                   \
          CmdListHandle cmdList,                                                                   \
          PipelineStageFlag stages,                                                                \
          size_t offset,                                                                           \
          size_t size,                                                                             \
          const void* pData ) override;                                                            \
                                                                                                   \
      TextureHandle createTexture( const TextureDescription& desc ) override;                      \
                                                                                                   \
      TextureHandle createTexture(                                                                 \
          CmdListHandle transferList,                                                              \
          const TextureDescription& desc,                                                          \
          const void* pTexels ) override;                                                          \
                                                                                                   \
      TextureHandle createTexture(                                                                 \
          CmdListHandle transferList,                                                              \
          const TextureDescription& desc,                                                          \
          uint32_t layerCount,                                                                     \
          const void* const* ppTexels ) override;                                                  \
                                                                                                   \
      VertexBufferHandle createVertexBuffer(                                                       \
          CmdListHandle transferList,                                                              \
          uint32_t count,                                                                          \
          uint32_t stride,                                                                         \
          const void* pVertices,                                                                   \
          const std::string_view name ) override;                                                  \
                                                                                                   \
      IndexBufferHandle createIndexBuffer(                                                         \
          CmdListHandle transferList,                                                              \
          uint32_t count,                                                                          \
          const void* pIndices,                                                                    \
          const std::string_view name ) override;                                                  \
                                                                                                   \
      BufferHandle createUniformBuffer( size_t size, const std::string_view name ) override;       \
                                                                                                   \
      BufferHandle createBuffer( size_t size, const std::string_view name ) override;              \
                                                                                                   \
      void copyToBuffer(                                                                           \
          BufferHandle bufferHandle,                                                               \
          const void* pData,                                                                       \
          size_t offset,                                                                           \
          size_t size ) override;                                                                  \
                                                                                                   \
      void destroyTexture( TextureHandle texHandle ) override;                                     \
      void destroyVertexBuffer( VertexBufferHandle bufferHandle ) override;                        \
      void destroyIndexBuffer( IndexBufferHandle bufferHandle ) override;                          \
      void destroyBuffer( BufferHandle bufferHandle ) override;                                    \
                                                                                                   \
      void prepareFrame() override;                                                                \
      void beginRendering( CmdListHandle cmdList ) override;                                       \
      void beginRendering(                                                                         \
          CmdListHandle cmdList,                                                                   \
          const FramebufferInfo& targetsInfo,                                                      \
          const std::vector<TextureHandle>& targets ) override;                                    \
      void nextPass( CmdListHandle cmdList ) override;                                             \
      void endRendering( CmdListHandle cmdList ) override;                                         \
      void draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ) override; \
      void drawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex )      \
          override;                                                                                \
      void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ )       \
          override;                                                                                \
      void presentFrame() override;                                                                \
                                                                                                   \
      void beginDebugRange(                                                                        \
          CmdListHandle cmdList,                                                                   \
          const char* name,                                                                        \
          const std::array<float, 4>& color ) override;                                            \
      void endDebugRange( CmdListHandle cmdList ) override;                                        \
      void insertDebugLabel(                                                                       \
          CmdListHandle cmdList,                                                                   \
          const char* name,                                                                        \
          const std::array<float, 4>& color ) override;                                            \
                                                                                                   \
     private:                                                                                      \
      NAMEImp* _imp;                                                                               \
   };                                                                                              \
   }