#include <cstdint>

/*
These are code generation macros for render backends to keep redundancies low when adding or
modifying functions in all the backends

Adding a function below will add a default empty function that will assert at runtime if
unimplemented in a render backend
*/

#define GRIS_RENDERBACKEND_DECLARATION( NAME )                                                     \
   namespace CYD                                                                                   \
   {                                                                                               \
   class Window;                                                                                   \
   class Framebuffer;                                                                              \
   class VertexList;                                                                               \
   struct RenderPassInfo;                                                                          \
   struct GraphicsPipelineInfo;                                                                    \
   struct ComputePipelineInfo;                                                                     \
   class NAME##Imp;                                                                                \
   }                                                                                               \
                                                                                                   \
   namespace CYD                                                                                   \
   {                                                                                               \
   GRIS_RENDERBACKEND_CLASS_NAME( NAME )                                                           \
   {                                                                                               \
     public:                                                                                       \
      GRIS_RENDERBACKEND_CONSTRUCTOR( NAME )                                                       \
      NON_COPIABLE( NAME );                                                                        \
      GRIS_RENDERBACKEND_DESTRUCTOR( NAME )                                                        \
                                                                                                   \
      GRIS_RENDERBACKEND_FUNC( bool initializeUIBackend(), return false; )                         \
      GRIS_RENDERBACKEND_FUNC( void uninitializeUIBackend(), )                                     \
      GRIS_RENDERBACKEND_FUNC( void drawUI( CmdListHandle cmdList ), )                             \
      GRIS_RENDERBACKEND_FUNC( void cleanup(), )                                                   \
      GRIS_RENDERBACKEND_FUNC( void reloadShaders(), )                                             \
      GRIS_RENDERBACKEND_FUNC( void waitUntilIdle(), )                                             \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          CmdListHandle createCommandList(                                                         \
              QueueUsageFlag usage, const std::string_view name, bool async, bool presentable ),   \
          return {}; )                                                                             \
      GRIS_RENDERBACKEND_FUNC( void submitCommandList( CmdListHandle cmdList ), );                 \
      GRIS_RENDERBACKEND_FUNC( void resetCommandList( CmdListHandle cmdList ), );                  \
      GRIS_RENDERBACKEND_FUNC( void waitOnCommandList( CmdListHandle cmdList ), );                 \
      GRIS_RENDERBACKEND_FUNC( void syncOnCommandList( CmdListHandle from, CmdListHandle to ), )   \
      GRIS_RENDERBACKEND_FUNC( void destroyCommandList( CmdListHandle cmdList ), );                \
      GRIS_RENDERBACKEND_FUNC( void syncOnSwapchain( CmdListHandle cmdList ), )                    \
      GRIS_RENDERBACKEND_FUNC( void syncToSwapchain( CmdListHandle cmdList ), )                    \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void setViewport( CmdListHandle cmdList, const Viewport& viewport ), )                   \
      GRIS_RENDERBACKEND_FUNC( void setScissor( CmdListHandle cmdList, const Rectangle& scissor ), \
                               ; )                                                                 \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo ), )       \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo ), )        \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle ), )       \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindIndexBuffer(                                                                    \
              CmdListHandle cmdList,                                                               \
              IndexBufferHandle bufferHandle,                                                      \
              IndexType type,                                                                      \
              uint32_t offset ), )                                                                 \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindTexture(                                                                        \
              CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding ), )  \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindTexture(                                                                        \
              CmdListHandle cmdList,                                                               \
              TextureHandle texHandle,                                                             \
              const SamplerInfo& sampler,                                                          \
              uint32_t set,                                                                        \
              uint32_t binding ), )                                                                \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindImage(                                                                          \
              CmdListHandle cmdList, TextureHandle texHandle, uint32_t set, uint32_t binding ), )  \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindBuffer(                                                                         \
              CmdListHandle cmdList,                                                               \
              BufferHandle bufferHandle,                                                           \
              uint32_t binding,                                                                    \
              uint32_t set,                                                                        \
              uint32_t offset,                                                                     \
              uint32_t range ), )                                                                  \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void bindUniformBuffer(                                                                  \
              CmdListHandle cmdList,                                                               \
              BufferHandle bufferHandle,                                                           \
              uint32_t binding,                                                                    \
              uint32_t set,                                                                        \
              uint32_t offset,                                                                     \
              uint32_t range ), )                                                                  \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void updateConstantBuffer(                                                               \
              CmdListHandle cmdList,                                                               \
              PipelineStageFlag stages,                                                            \
              size_t offset,                                                                       \
              size_t size,                                                                         \
              const void* pData ), )                                                               \
      GRIS_RENDERBACKEND_FUNC( TextureHandle createTexture( const TextureDescription& desc ),      \
                               return {}; )                                                        \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          TextureHandle createTexture(                                                             \
              CmdListHandle transferList, const TextureDescription& desc, const void* pTexels ),   \
          return {}; )                                                                             \
      GRIS_RENDERBACKEND_FUNC( TextureHandle createTexture(                                        \
                                   CmdListHandle transferList,                                     \
                                   const TextureDescription& desc,                                 \
                                   uint32_t layerCount,                                            \
                                   const void* const* ppTexels ),                                  \
                               return {}; )                                                        \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void generateMipmaps( CmdListHandle cmdList, TextureHandle texHandle ), )                \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          VertexBufferHandle createVertexBuffer( size_t size, const std::string_view name ),       \
          return {}; )                                                                             \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          IndexBufferHandle createIndexBuffer( size_t size, const std::string_view name ),         \
          return {}; )                                                                             \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          BufferHandle createUniformBuffer( size_t size, const std::string_view name ),            \
          return {}; )                                                                             \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          BufferHandle createBuffer( size_t size, const std::string_view name ), return {}; )      \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void uploadToBuffer(                                                                     \
              BufferHandle bufferHandle, const void* pData, const UploadToBufferInfo& info ), )    \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void uploadToVertexBuffer(                                                               \
              CmdListHandle transferList,                                                          \
              VertexBufferHandle bufferHandle,                                                     \
              const VertexList& vertices ), )                                                      \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void uploadToIndexBuffer(                                                                \
              CmdListHandle transferList,                                                          \
              IndexBufferHandle bufferHandle,                                                      \
              const void* pIndices,                                                                \
              const UploadToBufferInfo& info ), )                                                  \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void copyTexture(                                                                        \
              CmdListHandle cmdList,                                                               \
              TextureHandle srcTexHandle,                                                          \
              TextureHandle dstTexHandle,                                                          \
              const TextureCopyInfo& info ), )                                                     \
      GRIS_RENDERBACKEND_FUNC( void* addDebugTexture( TextureHandle texture, uint32_t layer = 0 ), \
                               return nullptr; )                                                   \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void updateDebugTexture( CmdListHandle cmdList, TextureHandle texture ), )               \
      GRIS_RENDERBACKEND_FUNC( void removeDebugTexture( void* texture ), )                         \
      GRIS_RENDERBACKEND_FUNC( void destroyTexture( TextureHandle texHandle ), )                   \
      GRIS_RENDERBACKEND_FUNC( void destroyVertexBuffer( VertexBufferHandle bufferHandle ), )      \
      GRIS_RENDERBACKEND_FUNC( void destroyIndexBuffer( IndexBufferHandle bufferHandle ), )        \
      GRIS_RENDERBACKEND_FUNC( void destroyBuffer( BufferHandle bufferHandle ), )                  \
      GRIS_RENDERBACKEND_FUNC( void beginFrame(), )                                                \
      GRIS_RENDERBACKEND_FUNC( void beginRendering( CmdListHandle cmdList ), )                     \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void beginRendering(                                                                     \
              CmdListHandle cmdList, const Framebuffer& fb, uint32_t layer = 0 ), )                \
      GRIS_RENDERBACKEND_FUNC( void nextPass( CmdListHandle cmdList ), )                           \
      GRIS_RENDERBACKEND_FUNC( void endRendering( CmdListHandle cmdList ), )                       \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex ), )            \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void drawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex ), )       \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void drawInstanced(                                                                      \
              CmdListHandle cmdList,                                                               \
              size_t vertexCount,                                                                  \
              size_t instanceCount,                                                                \
              size_t firstVertex,                                                                  \
              size_t firstInstance ), )                                                            \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void drawIndexedInstanced(                                                               \
              CmdListHandle cmdList,                                                               \
              size_t indexCount,                                                                   \
              size_t instanceCount,                                                                \
              size_t firstIndex,                                                                   \
              size_t firstInstance ), )                                                            \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ ),  \
          ; )                                                                                      \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void clearTexture(                                                                       \
              CmdListHandle cmdList, TextureHandle texHandle, const ClearValue& clearVal ), )      \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void copyToSwapchain( CmdListHandle cmdList, TextureHandle texHandle ), )                \
      GRIS_RENDERBACKEND_FUNC( void presentFrame(), )                                              \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void beginDebugRange(                                                                    \
              CmdListHandle cmdList, const char* name, const std::array<float, 4>& color ), )      \
      GRIS_RENDERBACKEND_FUNC( void endDebugRange( CmdListHandle cmdList ), )                      \
      GRIS_RENDERBACKEND_FUNC(                                                                     \
          void insertDebugLabel(                                                                   \
              CmdListHandle cmdList, const char* name, const std::array<float, 4>& color ), )      \
      GRIS_RENDERBACKEND_PIMPL( NAME )                                                             \
   };                                                                                              \
   }