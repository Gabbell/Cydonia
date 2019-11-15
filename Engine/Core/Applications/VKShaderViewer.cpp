#include <Core/Applications/VKShaderViewer.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Vulkan/Types.h>
#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/DeviceHerder.h>
#include <Core/Graphics/Vulkan/CommandBuffer.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/Buffer.h>

static constexpr char TITLE[] = "Vulkan Shader Viewer";

static cyd::PipelineInfo pipInfo;
static cyd::PipelineLayoutInfo pipLayoutInfo;

static cyd::Swapchain* swapchain = nullptr;
static cyd::Device* device       = nullptr;

cyd::VKShaderViewer::VKShaderViewer(
    uint32_t width,
    uint32_t height,
    const std::string& vertShader,
    const std::string& fragShader )
    : Application( width, height, TITLE ), _vertShader( vertShader ), _fragShader( fragShader )
{
}

void cyd::VKShaderViewer::preLoop()
{
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.usage      = ImageLayout::PRESENTATION;

   RenderPassInfo renderPassInfo = {};
   renderPassInfo.attachments.push_back( colorPresentation );

   SwapchainInfo scInfo = {};
   scInfo.extent        = _window->getExtent();
   scInfo.format        = PixelFormat::BGRA8_UNORM;
   scInfo.space         = ColorSpace::SRGB_NONLINEAR;
   scInfo.mode          = PresentMode::MAILBOX;

   pipLayoutInfo.ranges.push_back( { ShaderStage::FRAGMENT_STAGE, 0, sizeof( float ) } );

   pipInfo.renderPass = renderPassInfo;
   pipInfo.drawPrim   = DrawPrimitive::TRIANGLE_STRIPS;
   pipInfo.extent     = _window->getExtent();
   pipInfo.polyMode   = PolygonMode::FILL;
   pipInfo.shaders    = { _vertShader, _fragShader };
   pipInfo.pipLayout  = pipLayoutInfo;

   device    = _dh->getMainDevice();
   swapchain = device->createSwapchain( scInfo );

   // Quad
   const std::vector<Vertex> vertices = {
       { { -1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { -1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } } };

   // Staging vertices
   size_t verticesSize = sizeof( vertices[0] ) * vertices.size();
   auto vertexStaging  = device->createStagingBuffer( verticesSize );
   vertexStaging->mapMemory( (void*)vertices.data(), verticesSize );

   // Uploading vertices to device memory
   _vertexBuffer =
       device->createDeviceBuffer( verticesSize, BufferUsage::TRANSFER_DST | BufferUsage ::VERTEX );

   auto transferCmds = device->createCommandBuffer( QueueUsage::TRANSFER );
   transferCmds->startRecording();
   transferCmds->copyBuffer( vertexStaging, _vertexBuffer );
   transferCmds->endRecording();
   transferCmds->submit();
   transferCmds->waitForCompletion();
}

void cyd::VKShaderViewer::drawFrame( double deltaTime )
{
   // Push constant
   static float currentTime = 0;
   currentTime += static_cast<float>( deltaTime );

   // Drawing in the swapchain
   auto cmds = device->createCommandBuffer( QueueUsage::GRAPHICS, true );

   Extent extent = _window->getExtent();

   cmds->startRecording();
   cmds->bindPipeline( pipInfo );
   cmds->updatePushConstants( pipLayoutInfo.ranges[0], &currentTime );
   cmds->bindVertexBuffer( _vertexBuffer );
   cmds->setViewport( extent.width, extent.height );
   cmds->beginPass( swapchain );
   cmds->draw( 4 );
   cmds->endPass();
   cmds->endRecording();
   cmds->submit();

   swapchain->present();

   device->cleanup();
}

cyd::VKShaderViewer::~VKShaderViewer() {}
