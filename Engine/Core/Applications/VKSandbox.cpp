#include <Core/Applications/VKSandbox.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Vulkan/Types.h>
#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/DeviceHerder.h>
#include <Core/Graphics/Vulkan/CommandBuffer.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/Buffer.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

static constexpr char TITLE[] = "Vulkan Sandbox";

static cyd::PipelineInfo pipInfo;
static cyd::PipelineLayoutInfo pipLayoutInfo;

static cyd::Swapchain* swapchain = nullptr;
static cyd::Device* device       = nullptr;

struct UBO
{
   glm::mat4 mv;
   glm::mat4 proj;
};

cyd::VKSandbox::VKSandbox( uint32_t width, uint32_t height ) : Application( width, height, TITLE )
{
}

void cyd::VKSandbox::preLoop()
{
   device = _dh->getMainDevice();

   // Quad
   const std::vector<Vertex> vertices = {
       { { -0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 0.0f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } } };

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

   // Creating swapchain
   SwapchainInfo scInfo = {};
   scInfo.extent        = _window->getExtent();
   scInfo.format        = PixelFormat::BGRA8_UNORM;
   scInfo.space         = ColorSpace::SRGB_NONLINEAR;
   scInfo.mode          = PresentMode::MAILBOX;

   swapchain = device->createSwapchain( scInfo );

   // Creating pipeline
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.usage      = AttachmentUsage::PRESENTATION;

   RenderPassInfo renderPassInfo = {};
   renderPassInfo.attachments.push_back( colorPresentation );

   ShaderObjectInfo uboInfo;
   uboInfo.size    = sizeof( UBO );
   uboInfo.usage   = BufferUsage::UNIFORM;
   uboInfo.stages  = ShaderStage::VERTEX_STAGE;
   uboInfo.binding = 0;

   pipLayoutInfo.descSetLayout.shaderObjects.push_back( uboInfo );

   pipInfo.renderPass = renderPassInfo;
   pipInfo.drawPrim   = DrawPrimitive::TRIANGLES;
   pipInfo.extent     = _window->getExtent();
   pipInfo.polyMode   = PolygonMode::FILL;
   pipInfo.shaders    = { "default_vert", "default_frag" };
   pipInfo.pipLayout  = pipLayoutInfo;

   _uboBuffer = device->createUniformBuffer(
       BufferUsage::TRANSFER_DST, uboInfo, pipLayoutInfo.descSetLayout );

   transferCmds->waitForCompletion();
}

void cyd::VKSandbox::drawFrame( double deltaTime )
{
   static float currentTime = 0;
   currentTime += static_cast<float>( deltaTime );

   Extent extent = _window->getExtent();
   auto cmds     = device->createCommandBuffer( QueueUsage::GRAPHICS | QueueUsage::TRANSFER, true );

   // Generating MVP
   UBO mvp;
   mvp.mv   = glm::rotate( (float)currentTime, glm::vec3( 0.0f, 0.0f, 1.0f ) );
   mvp.proj = glm::orthoZO( -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f );

   // Update MVP UBO
   _uboBuffer->mapMemory( &mvp, sizeof( UBO ) );

   // Drawing in the swapchain
   cmds->startRecording();
   cmds->bindPipeline( pipInfo );
   cmds->setViewport( extent.width, extent.height );
   cmds->bindVertexBuffer( _vertexBuffer );
   cmds->bindBuffer( _uboBuffer );
   cmds->beginPass( swapchain );
   cmds->draw( 3 );
   cmds->endPass();
   cmds->endRecording();
   cmds->submit();

   swapchain->present();

   device->cleanup();
}

cyd::VKSandbox::~VKSandbox() {}
