#include <Core/VkApplication.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Vulkan/Types.h>
#include <Core/Graphics/Vulkan/Instance.h>
#include <Core/Graphics/Vulkan/Surface.h>
#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/DeviceHerder.h>
#include <Core/Graphics/Vulkan/CommandBuffer.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/Buffer.h>

static cyd::PipelineInfo pipInfo;

static cyd::Swapchain* swapchain = nullptr;
static cyd::Device* device       = nullptr;

cyd::VkApplication::VkApplication( uint32_t width, uint32_t height, const std::string& title )
    : Application( width, height, title )
{
   _instance = std::make_unique<Instance>( *_window );
   _surface  = std::make_unique<Surface>( *_instance, *_window );
   _dh       = std::make_unique<DeviceHerder>( *_instance, *_window, *_surface );

   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.usage      = AttachmentUsage::PRESENTATION;

   RenderPassInfo renderPassInfo = {};
   renderPassInfo.attachments.push_back( colorPresentation );

   SwapchainInfo scInfo = {};
   scInfo.extent        = _window->getExtent();
   scInfo.format        = PixelFormat::BGRA8_UNORM;
   scInfo.space         = ColorSpace::SRGB_NONLINEAR;
   scInfo.mode          = PresentMode::MAILBOX;

   PipelineLayoutInfo pipLayout = {};
   pipInfo.renderPass           = renderPassInfo;
   pipInfo.drawPrim             = DrawPrimitive::TRIANGLES;
   pipInfo.extent               = _window->getExtent();
   pipInfo.polyMode             = PolygonMode::FILL;
   pipInfo.shaders              = { "default_vert.spv", "default_frag.spv" };
   pipInfo.pipLayout            = pipLayout;

   device    = _dh->getMainDevice();
   swapchain = device->createSwapchain( scInfo );

   // Triangle
   const std::vector<Vertex> vertices = {
       { { 0.0f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
       { { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } } };

   // Staging vertices
   size_t verticesSize = sizeof( vertices[0] ) * vertices.size();
   auto vertexStage    = device->createStagingBuffer( verticesSize, BufferUsage::TRANSFER_SRC );
   vertexStage->mapMemory( (void*)vertices.data(), verticesSize );

   // Uploading vertices to device memory
   _vertexBuffer =
       device->createBuffer( verticesSize, BufferUsage::TRANSFER_DST | BufferUsage ::VERTEX );

   auto transferCmds = device->createCommandBuffer( QueueUsage::TRANSFER );
   transferCmds->startRecording();
   transferCmds->copyBuffer( vertexStage, _vertexBuffer );
   transferCmds->endRecording();
   transferCmds->submit();
   transferCmds->waitForCompletion();
}

void cyd::VkApplication::drawFrame( double deltaTime )
{
   // Drawing in the swapchain
   auto cmds = device->createCommandBuffer( QueueUsage::GRAPHICS, true );

   Extent extent = _window->getExtent();

   cmds->startRecording();
   cmds->bindPipeline( pipInfo );
   cmds->bindVertexBuffer( _vertexBuffer );
   cmds->setViewport( extent.width, extent.height );
   cmds->beginPass( swapchain );
   cmds->draw();
   cmds->endPass();
   cmds->endRecording();

   // TODO Make it so that submit and present is different? i.e. present does NOT submit
   swapchain->present();

   device->cleanup();
}

cyd::VkApplication::~VkApplication() {}
