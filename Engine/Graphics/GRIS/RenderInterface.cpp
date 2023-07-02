#include <Graphics/GRIS/RenderInterface.h>

#include <Common/Assert.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Utility/GraphicsIO.h>
#include <Graphics/GRIS/Backends/VKRenderBackend.h>
#include <Graphics/GRIS/Backends/D3D12RenderBackend.h>

#include <Profiling.h>

#include <ThirdParty/ImGui/imgui.h>

#include <cstdio>

namespace CYD::GRIS
{
static RenderBackend* b = nullptr;

// =================================================================================================
// Initialization
//
bool InitRenderBackend( API api, const Window& window )
{
   delete b;

   switch( api )
   {
      case API::VK:
         b = new VKRenderBackend( window );
         return true;
      case API::D3D12:
         b = new D3D12RenderBackend( window );
         return true;
      case API::D3D11:
         return false;
      case API::GL:
         return false;
      case API::MTL:
         return false;
   }

   return false;
}

void UninitRenderBackend() { delete b; }

bool InitializeUIBackend()
{
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui::StyleColorsDark();

   return b->initializeUIBackend();
}

void UninitializeUIBackend()
{
   b->uninitializeUIBackend();

   ImGui::DestroyContext();
}

void DrawUI( CmdListHandle cmdList )
{
   CYD_TRACE( "Draw UI" );
   b->drawUI( cmdList );
}

void RenderBackendCleanup()
{
   CYD_TRACE( "Render Backend Cleanup" );
   b->cleanup();
}

void WaitUntilIdle() { b->waitUntilIdle(); }

// =================================================================================================
// Command Buffers/Lists
//
CmdListHandle GetMainCommandList() { return b->getMainCommandList(); }
CmdListHandle
CreateCommandList( QueueUsageFlag usage, const std::string_view name, bool presentable )
{
   return b->createCommandList( usage, name, presentable );
}

void SubmitCommandList( CmdListHandle cmdList ) { b->submitCommandList( cmdList ); }
void SubmitCommandLists( const std::vector<CmdListHandle>& /*cmdLists*/ ) {}
void ResetCommandList( CmdListHandle cmdList ) { b->resetCommandList( cmdList ); }
void WaitOnCommandList( CmdListHandle cmdList ) { b->waitOnCommandList( cmdList ); }
void SyncOnCommandList( CmdListHandle from, CmdListHandle to ) { b->syncOnCommandList( from, to ); }
void DestroyCommandList( CmdListHandle cmdList ) { b->destroyCommandList( cmdList ); }

void SyncOnSwapchain( CmdListHandle cmdList ) { b->syncOnSwapchain( cmdList ); }
void SyncToSwapchain( CmdListHandle cmdList ) { b->syncToSwapchain( cmdList ); }

// =================================================================================================
// Pipeline Specification
//
void SetViewport( CmdListHandle cmdList, const Viewport& viewport )
{
   b->setViewport( cmdList, viewport );
}

void SetScissor( CmdListHandle cmdList, const Rectangle& scissor )
{
   b->setScissor( cmdList, scissor );
}

void BindPipeline( CmdListHandle cmdList, const GraphicsPipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void BindPipeline( CmdListHandle cmdList, const ComputePipelineInfo& pipInfo )
{
   b->bindPipeline( cmdList, pipInfo );
}

void BindPipeline( CmdListHandle cmdList, const PipelineInfo* pPipInfo )
{
   if( pPipInfo )
   {
      switch( pPipInfo->type )
      {
         case PipelineType::GRAPHICS:
            b->bindPipeline( cmdList, *static_cast<const GraphicsPipelineInfo*>( pPipInfo ) );
            break;
         case PipelineType::COMPUTE:
            b->bindPipeline( cmdList, *static_cast<const ComputePipelineInfo*>( pPipInfo ) );
            break;
      }
   }
}

void BindPipeline( CmdListHandle cmdList, PipelineIndex index )
{
   const PipelineInfo* pPipInfo = StaticPipelines::Get( index );
   BindPipeline( cmdList, pPipInfo );
}

template <>
void BindVertexBuffer<Vertex>( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
{
   b->bindVertexBuffer( cmdList, bufferHandle );
}

template <>
void BindIndexBuffer<uint16_t>(
    CmdListHandle cmdList,
    IndexBufferHandle bufferHandle,
    uint32_t offset )
{
   b->bindIndexBuffer( cmdList, bufferHandle, IndexType::UNSIGNED_INT16, offset );
}

template <>
void BindIndexBuffer<uint32_t>(
    CmdListHandle cmdList,
    IndexBufferHandle bufferHandle,
    uint32_t offset )
{
   b->bindIndexBuffer( cmdList, bufferHandle, IndexType::UNSIGNED_INT32, offset );
}

void BindTexture( CmdListHandle cmdList, TextureHandle texHandle, uint32_t binding, uint32_t set )
{
   b->bindTexture( cmdList, texHandle, binding, set );
}

void BindTexture(
    CmdListHandle cmdList,
    TextureHandle texHandle,
    const SamplerInfo& sampler,
    uint32_t binding,
    uint32_t set )
{
   b->bindTexture( cmdList, texHandle, sampler, binding, set );
}

void BindImage( CmdListHandle cmdList, TextureHandle texHandle, uint32_t binding, uint32_t set )
{
   b->bindImage( cmdList, texHandle, binding, set );
}

void BindBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set,
    uint32_t offset,
    uint32_t range )
{
   b->bindBuffer( cmdList, bufferHandle, binding, set, offset, range );
}

void BindUniformBuffer(
    CmdListHandle cmdList,
    BufferHandle bufferHandle,
    uint32_t binding,
    uint32_t set,
    uint32_t offset,
    uint32_t range )
{
   b->bindUniformBuffer( cmdList, bufferHandle, binding, set, offset, range );
}

void UpdateConstantBuffer(
    CmdListHandle cmdList,
    PipelineStageFlag stages,
    size_t offset,
    size_t size,
    const void* pData )
{
   b->updateConstantBuffer( cmdList, stages, offset, size, pData );
}

// =================================================================================================
// Resources
//
TextureHandle CreateTexture( const TextureDescription& desc ) { return b->createTexture( desc ); }

static TextureHandle LoadImageFromStorage(
    CmdListHandle transferList,
    const TextureDescription& inputDesc,
    uint32_t layerCount,
    const std::string* paths )
{
   CYD_ASSERT(
       layerCount <= inputDesc.layers &&
       "VKRenderBackend:: Number of textures could not fit in number of layers" );

   CYD_ASSERT(
       inputDesc.width == 0 && inputDesc.height == 0 && inputDesc.size == 0 &&
       "VKRenderBackend: Created a texture with a path but specified dimensions" );

   std::vector<void*> imageData;
   int prevWidth     = 0;
   int prevHeight    = 0;
   int prevLayerSize = 0;
   int width         = 0;
   int height        = 0;
   int layerSize     = 0;
   int totalSize     = 0;

   for( uint32_t i = 0; i < layerCount; ++i )
   {
      imageData.push_back(
          GraphicsIO::LoadImage( paths[i], inputDesc.format, width, height, layerSize ) );

      if( !imageData.back() )
      {
         return Handle();
      }

      // Sanity check
      if( prevWidth == 0 ) prevWidth = width;
      if( prevHeight == 0 ) prevHeight = height;
      if( prevLayerSize == 0 ) prevLayerSize = layerSize;

      CYD_ASSERT(
          prevWidth == width && prevHeight == height && prevLayerSize == layerSize &&
          "VKRenderBackend: Dimension mismatch" );

      prevWidth     = width;
      prevHeight    = height;
      prevLayerSize = layerSize;

      totalSize += layerSize;
   }

   // Description used to create the texture with the actual dimensions
   TextureDescription newDesc = inputDesc;
   newDesc.width              = width;
   newDesc.height             = height;
   newDesc.size               = totalSize;

   TextureHandle texHandle = b->createTexture(
       transferList, newDesc, static_cast<uint32_t>( imageData.size() ), imageData.data() );

   for( uint32_t i = 0; i < imageData.size(); ++i )
   {
      GraphicsIO::FreeImage( imageData[i] );
   }

   return texHandle;
}

TextureHandle
CreateTexture( CmdListHandle transferList, const TextureDescription& desc, const std::string& path )
{
   return LoadImageFromStorage( transferList, desc, 1, &path );
}

TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    const std::vector<std::string>& paths )
{
   return LoadImageFromStorage(
       transferList, desc, static_cast<uint32_t>( paths.size() ), paths.data() );
}

TextureHandle
CreateTexture( CmdListHandle transferList, const TextureDescription& desc, const void* pTexels )
{
   return b->createTexture( transferList, desc, pTexels );
}

TextureHandle CreateTexture(
    CmdListHandle transferList,
    const TextureDescription& desc,
    uint32_t layerCount,
    const void** ppTexels )
{
   return b->createTexture( transferList, desc, layerCount, ppTexels );
}

VertexBufferHandle CreateVertexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    uint32_t stride,
    const void* pVertices,
    const std::string_view name )
{
   return b->createVertexBuffer( transferList, count, stride, pVertices, name );
}

IndexBufferHandle CreateIndexBuffer(
    CmdListHandle transferList,
    uint32_t count,
    const void* pIndices,
    const std::string_view name )
{
   return b->createIndexBuffer( transferList, count, pIndices, name );
}

BufferHandle CreateUniformBuffer( size_t size, const std::string_view name )
{
   return b->createUniformBuffer( size, name );
}

BufferHandle CreateBuffer( size_t size, const std::string_view name )
{
   return b->createBuffer( size, name );
}

void* AddDebugTexture( TextureHandle texture ) { return b->addDebugTexture( texture ); }
void UpdateDebugTexture( CmdListHandle cmdList, TextureHandle textureHandle )
{
   return b->updateDebugTexture( cmdList, textureHandle );
};
void RemoveDebugTexture( void* texture ) { b->removeDebugTexture( texture ); }

void CopyToBuffer( BufferHandle bufferHandle, const void* pData, size_t offset, size_t size )
{
   return b->copyToBuffer( bufferHandle, pData, offset, size );
}

void DestroyTexture( TextureHandle texHandle ) { b->destroyTexture( texHandle ); }

void DestroyVertexBuffer( VertexBufferHandle bufferHandle )
{
   b->destroyVertexBuffer( bufferHandle );
}

void DestroyIndexBuffer( IndexBufferHandle bufferHandle ) { b->destroyIndexBuffer( bufferHandle ); }

void DestroyBuffer( BufferHandle bufferHandle ) { b->destroyBuffer( bufferHandle ); }

// =================================================================================================
// Drawing
//
void PrepareFrame()
{
   CYD_TRACE( "Prepare Frame" );
   b->prepareFrame();
}

void BeginRendering( CmdListHandle cmdList ) { b->beginRendering( cmdList ); }

void BeginRendering(
    CmdListHandle cmdList,
    const FramebufferInfo& targetsInfo,
    const std::vector<TextureHandle>& targets )
{
   b->beginRendering( cmdList, targetsInfo, targets );
}

void NextPass( CmdListHandle cmdList ) { b->nextPass( cmdList ); }

void EndRendering( CmdListHandle cmdList ) { b->endRendering( cmdList ); }

void Draw( CmdListHandle cmdList, size_t vertexCount, size_t firstVertex )
{
   b->draw( cmdList, vertexCount, firstVertex );
}

void DrawIndexed( CmdListHandle cmdList, size_t indexCount, size_t firstIndex )
{
   b->drawIndexed( cmdList, indexCount, firstIndex );
}

void DrawInstanced(
    CmdListHandle cmdList,
    size_t vertexCount,
    size_t instanceCount,
    size_t firstVertex,
    size_t firstInstance )
{
   b->drawInstanced( cmdList, vertexCount, instanceCount, firstVertex, firstInstance );
}

void DrawIndexedInstanced(
    CmdListHandle cmdList,
    size_t indexCount,
    size_t instanceCount,
    size_t firstIndex,
    size_t firstInstance )
{
   b->drawIndexedInstanced( cmdList, indexCount, instanceCount, firstIndex, firstInstance );
}

void Dispatch( CmdListHandle cmdList, uint32_t workX, uint32_t workY, uint32_t workZ )
{
   b->dispatch( cmdList, workX, workY, workZ );
}

void PresentFrame()
{
   CYD_TRACE( "Present Frame" );
   b->presentFrame();
}

void BeginDebugRange( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color )
{
   b->beginDebugRange( cmdList, name, color );
}

void EndDebugRange( CmdListHandle cmdList ) { b->endDebugRange( cmdList ); }

void InsertDebugLabel( CmdListHandle cmdList, const char* name, const std::array<float, 4>& color )
{
   b->insertDebugLabel( cmdList, name, color );
}
}
