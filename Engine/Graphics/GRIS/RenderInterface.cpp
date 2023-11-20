#include <Graphics/GRIS/RenderInterface.h>

#include <Common/Assert.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/Framebuffer.h>
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
         // b = new D3D12RenderBackend( window );
         return true;
      case API::D3D11:
         return false;
      case API::GL:
         // b = new GLRenderBackend( window );
         return false;
      case API::MTL:
         return false;
      default:
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
   CYD_TRACE();
   b->drawUI( cmdList );
}

void RenderBackendCleanup()
{
   CYD_TRACE();
   b->cleanup();
}

void ReloadShaders()
{
   CYD_TRACE();
   printf( "Reloading Shaders\n" );
   b->reloadShaders();
}

void WaitUntilIdle() { b->waitUntilIdle(); }

// =================================================================================================
// Command Buffers/Lists
//
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

void BindVertexBuffer( CmdListHandle cmdList, VertexBufferHandle bufferHandle )
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

TextureHandle
CreateTexture( CmdListHandle cmdList, const TextureDescription& desc, const void* pTexels )
{
   return b->createTexture( cmdList, desc, pTexels );
}

TextureHandle CreateTexture(
    CmdListHandle cmdList,
    const TextureDescription& desc,
    uint32_t layerCount,
    const void** ppTexels )
{
   return b->createTexture( cmdList, desc, layerCount, ppTexels );
}

void GenerateMipmaps( CmdListHandle cmdList, TextureHandle texHandle )
{
   b->generateMipmaps( cmdList, texHandle );
}

VertexBufferHandle CreateVertexBuffer( size_t size, const std::string_view name )
{
   return b->createVertexBuffer( size, name );
}

IndexBufferHandle CreateIndexBuffer( size_t size, const std::string_view name )
{
   return b->createIndexBuffer( size, name );
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

void UploadToBuffer( BufferHandle bufferHandle, const void* pData, const UploadToBufferInfo& info )
{
   return b->uploadToBuffer( bufferHandle, pData, info );
}

void UploadToVertexBuffer(
    CmdListHandle cmdList,
    VertexBufferHandle bufferHandle,
    const VertexList& vertices )
{
   b->uploadToVertexBuffer( cmdList, bufferHandle, vertices );
}

void UploadToIndexBuffer(
    CmdListHandle cmdList,
    IndexBufferHandle bufferHandle,
    const void* pIndices,
    const UploadToBufferInfo& info )
{
   b->uploadToIndexBuffer( cmdList, bufferHandle, pIndices, info );
}

void CopyTexture(
    CmdListHandle cmdList,
    TextureHandle srcTexHandle,
    TextureHandle dstTexHandle,
    const TextureCopyInfo& info )
{
   return b->copyTexture( cmdList, srcTexHandle, dstTexHandle, info );
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
void BeginFrame()
{
   CYD_TRACE();
   b->beginFrame();
}

void BeginRendering( CmdListHandle cmdList ) { b->beginRendering( cmdList ); }

void BeginRendering( CmdListHandle cmdList, const Framebuffer& fb )
{
   b->beginRendering( cmdList, fb );
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

void ClearTexture( CmdListHandle cmdList, TextureHandle texHandle, const ClearValue& clearVal )
{
   b->clearTexture( cmdList, texHandle, clearVal );
}

void CopyToSwapchain( CmdListHandle cmdList, TextureHandle texHandle )
{
   b->copyToSwapchain( cmdList, texHandle );
}

void PresentFrame()
{
   CYD_TRACE();
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
