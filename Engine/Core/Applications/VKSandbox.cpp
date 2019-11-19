#include <Core/Applications/VKSandbox.h>

#include <Core/Window/GLFWWindow.h>

#include <Core/Input/CameraController.h>
#include <Core/Input/InputInterpreter.h>

#include <Core/Graphics/Scene/SceneContext.h>
#include <Core/Graphics/Scene/Camera.h>

#include <Core/Graphics/Vulkan/Types.h>
#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/DeviceHerder.h>
#include <Core/Graphics/Vulkan/CommandBuffer.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/Buffer.h>
#include <Core/Graphics/Vulkan/Texture.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>

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
   // Making a controller for the scene's camera. Passing the camera through here is a bit awkward
   _controller = std::make_unique<CameraController>( _sceneContext->getCamera() );

   _inputInterpreter->addController( *_controller );

   device = _dh->getMainDevice();

   // Creating pipeline
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.usage      = ImageLayout::PRESENTATION;

   RenderPassInfo renderPassInfo = {};
   renderPassInfo.attachments.push_back( colorPresentation );

   ShaderObjectInfo uboInfo;
   uboInfo.size    = sizeof( UBO );
   uboInfo.type    = ShaderObjectType::UNIFORM;
   uboInfo.stages  = ShaderStage::VERTEX_STAGE;
   uboInfo.binding = 0;

   ShaderObjectInfo texInfo;
   texInfo.type    = ShaderObjectType::COMBINED_IMAGE_SAMPLER;
   texInfo.stages  = ShaderStage::FRAGMENT_STAGE;
   texInfo.binding = 1;

   pipLayoutInfo.descSetLayout.shaderObjects.push_back( uboInfo );
   pipLayoutInfo.descSetLayout.shaderObjects.push_back( texInfo );

   pipInfo.renderPass = renderPassInfo;
   pipInfo.drawPrim   = DrawPrimitive::TRIANGLE_STRIPS;
   pipInfo.extent     = _window->getExtent();
   pipInfo.polyMode   = PolygonMode::FILL;
   pipInfo.shaders    = { "defaultTex_vert", "defaultTex_frag" };
   pipInfo.pipLayout  = pipLayoutInfo;

   // Triangle
   const std::vector<Vertex> vertices = {
       { { -0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { 0.0f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
       { { 0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } } };

   // Placeholder texture
   std::array<uint32_t, 4> texData = { 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF };
   size_t texSize                  = sizeof( texData[0] ) * texData.size();

   TextureDescription texDesc;
   texDesc.size   = texSize;
   texDesc.width  = 2;
   texDesc.height = 2;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   _texture = device->createTexture( texDesc, texInfo, pipLayoutInfo.descSetLayout );

   size_t verticesSize = sizeof( vertices[0] ) * vertices.size();
   _vertexBuffer =
       device->createDeviceBuffer( verticesSize, BufferUsage::TRANSFER_DST | BufferUsage ::VERTEX );

   _uboBuffer = device->createUniformBuffer(
       BufferUsage::TRANSFER_DST, uboInfo, pipLayoutInfo.descSetLayout );

   // Staging
   auto vertexStaging = device->createStagingBuffer( verticesSize );
   auto texStaging    = device->createStagingBuffer( texSize );

   vertexStaging->mapMemory( (void*)vertices.data(), verticesSize );
   texStaging->mapMemory( texData.data(), texSize );

   // Uploading vertices to device memory
   auto transferCmds = device->createCommandBuffer( QueueUsage::TRANSFER );
   transferCmds->startRecording();
   transferCmds->copyBuffer( vertexStaging, _vertexBuffer );
   transferCmds->uploadBufferToTex( texStaging, _texture );
   transferCmds->endRecording();
   transferCmds->submit();

   // Creating swapchain
   SwapchainInfo scInfo = {};
   scInfo.extent        = _window->getExtent();
   scInfo.format        = PixelFormat::BGRA8_UNORM;
   scInfo.space         = ColorSpace::SRGB_NONLINEAR;
   scInfo.mode          = PresentMode::MAILBOX;

   swapchain = device->createSwapchain( scInfo );

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
   mvp.mv   = _sceneContext->getCamera().getViewMatrix();
   mvp.proj = _sceneContext->getCamera().getProjectionMatrix();

   // Update MVP UBO
   _uboBuffer->mapMemory( &mvp, sizeof( UBO ) );

   // Drawing in the swapchain
   cmds->startRecording();
   cmds->bindPipeline( pipInfo );
   cmds->setViewport( _sceneContext->getCamera().getViewport() );
   cmds->bindVertexBuffer( _vertexBuffer );
   cmds->bindBuffer( _uboBuffer );
   cmds->bindTexture( _texture );
   cmds->beginPass( swapchain );
   cmds->draw( 3 );
   cmds->endPass();
   cmds->endRecording();
   cmds->submit();

   swapchain->present();

   device->cleanup();
}

cyd::VKSandbox::~VKSandbox() {}
