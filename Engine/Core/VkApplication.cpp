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

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

static cyd::PipelineInfo pipInfo;
static cyd::PipelineLayoutInfo pipLayoutInfo;

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

   pipLayoutInfo.ranges.push_back( { ShaderStage::FRAGMENT_STAGE, 0, sizeof( float ) } );

   pipInfo.renderPass = renderPassInfo;
   pipInfo.drawPrim   = DrawPrimitive::TRIANGLES;
   pipInfo.extent     = _window->getExtent();
   pipInfo.polyMode   = PolygonMode::FILL;
   pipInfo.shaders    = { "passthrough_vert.spv", "proteanclouds_frag.spv" };
   pipInfo.pipLayout  = pipLayoutInfo;

   device    = _dh->getMainDevice();
   swapchain = device->createSwapchain( scInfo );

   // Quad
   const std::vector<Vertex> vertices = {
       { { -1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { -1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { -1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } } };

   // Staging vertices
   size_t verticesSize = sizeof( vertices[0] ) * vertices.size();
   auto vertexStaging  = device->createStagingBuffer( verticesSize, BufferUsage::TRANSFER_SRC );
   vertexStaging->mapMemory( (void*)vertices.data(), verticesSize );

   // Uploading vertices to device memory
   _vertexBuffer =
       device->createBuffer( verticesSize, BufferUsage::TRANSFER_DST | BufferUsage ::VERTEX );

   auto transferCmds = device->createCommandBuffer( QueueUsage::TRANSFER );
   transferCmds->startRecording();
   transferCmds->copyBuffer( vertexStaging, _vertexBuffer );
   transferCmds->endRecording();
   transferCmds->submit();
   transferCmds->waitForCompletion();
}

void cyd::VkApplication::drawFrame( double deltaTime )
{
   printf( "FPS:%d\n", static_cast<uint32_t>( 1.0 / deltaTime ) );
   static float currentTime = 0;
   currentTime += static_cast<float>( deltaTime );

   // Generating MVP
   // glm::mat4 proj  = glm::orthoZO( -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f );
   // glm::mat4 mv    = glm::rotate( (float)currentTime, glm::vec3( 0.0f, 1.0f, 0.0f ) );
   // glm::mat4 mvp[] = { mv, proj };

   // Drawing in the swapchain
   auto cmds = device->createCommandBuffer( QueueUsage::GRAPHICS, true );

   Extent extent = _window->getExtent();

   cmds->startRecording();
   cmds->bindPipeline( pipInfo );
   cmds->updatePushConstants( pipLayoutInfo.ranges[0], &currentTime );
   cmds->bindVertexBuffer( _vertexBuffer );
   cmds->setViewport( extent.width, extent.height );
   cmds->beginPass( swapchain );
   cmds->draw( 6 );
   cmds->endPass();
   cmds->endRecording();
   cmds->submit();

   swapchain->present();

   device->cleanup();
}

cyd::VkApplication::~VkApplication() {}
