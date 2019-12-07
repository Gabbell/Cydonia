#include <Applications/VKSandbox.h>

#include <Window/GLFWWindow.h>

#include <HID/FreeCameraController.h>
#include <HID/InputInterpreter.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/Scene/Scene.h>
#include <Graphics/Scene/Camera.h>

#include <Handles/Handle.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>

struct UBO
{
   glm::mat4 model = glm::mat4( 1.0f );
   glm::mat4 view  = glm::mat4( 1.0f );
   glm::mat4 proj  = glm::mat4( 1.0f );
};

namespace cyd
{
static constexpr char TITLE[] = "Vulkan Sandbox";
static PipelineInfo pipInfo;
static PipelineLayoutInfo pipLayoutInfo;

VKSandbox::VKSandbox( uint32_t width, uint32_t height ) : VKApplication( width, height, TITLE )
{
   // Creating pipeline
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.layout     = ImageLayout::PRESENTATION;

   Attachment depthPresentation = {};
   depthPresentation.format     = PixelFormat::D32_SFLOAT;
   depthPresentation.loadOp     = LoadOp::CLEAR;
   depthPresentation.storeOp    = StoreOp::DONT_CARE;
   depthPresentation.type       = AttachmentType::DEPTH_STENCIL;
   depthPresentation.layout     = ImageLayout::DEPTH_STENCIL;

   RenderPassInfo renderPassInfo = {};
   renderPassInfo.attachments.push_back( colorPresentation );
   renderPassInfo.attachments.push_back( depthPresentation );

   ShaderObjectInfo uboInfo;
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
   pipInfo.drawPrim   = DrawPrimitive::TRIANGLES;
   pipInfo.extent     = _window->getExtent();
   pipInfo.polyMode   = PolygonMode::FILL;
   pipInfo.shaders    = {"defaultTex_vert", "defaultTex_frag"};
   pipInfo.pipLayout  = pipLayoutInfo;

   // Making a controller for the scene's camera. Passing the camera through here is a bit awkward
   _controller = std::make_unique<FreeCameraController>( _scene->getCamera() );

   _inputInterpreter->addController( *_controller );
}

void VKSandbox::preLoop()
{
   CmdListHandle transferList = createCommandList( QueueUsage::TRANSFER );

   _uboBuffer = createUniformBuffer(
       sizeof( UBO ), pipLayoutInfo.descSetLayout.shaderObjects[0], pipLayoutInfo.descSetLayout );

   // Triangle
   const std::vector<Vertex> vertices = {
       {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}};

   // Placeholder texture
   std::array<uint32_t, 4> texData = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF};
   size_t texSize                  = sizeof( texData[0] ) * texData.size();

   TextureDescription texDesc;
   texDesc.size   = texSize;
   texDesc.width  = 2;
   texDesc.height = 2;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   startRecordingCommandList( transferList );

   _vertexBuffer =
       createVertexBuffer( transferList, vertices.size(), sizeof( Vertex ), vertices.data() );

   _texture = createTexture(
       transferList,
       texDesc,
       pipLayoutInfo.descSetLayout.shaderObjects[1],
       pipLayoutInfo.descSetLayout,
       texData.data() );

   endRecordingCommandList( transferList );

   submitCommandList( transferList );
   waitOnCommandList( transferList );
   destroyCommandList( transferList );
}

void VKSandbox::drawNextFrame( double )
{
   // Generating MVP
   UBO mvp;
   mvp.model = glm::scale( glm::vec3( 1.0f, 1.0f, 1.0f ) );
   mvp.view  = _scene->getCamera().getViewMatrix();
   mvp.proj  = _scene->getCamera().getProjectionMatrix();

   mapUniformBufferMemory( _uboBuffer, &mvp );

   CmdListHandle cmdList = createCommandList( QueueUsage::GRAPHICS, true );

   startRecordingCommandList( cmdList );

   bindPipeline( cmdList, pipInfo );
   setViewport( cmdList, {{0.0f, 0.0f}, _window->getExtent()} );
   bindTexture( cmdList ,_texture );
   bindVertexBuffer( cmdList, _vertexBuffer );
   bindUniformBuffer( cmdList, _uboBuffer );
   beginRenderPass( cmdList );
   drawFrame( cmdList, 3 );
   endRenderPass( cmdList );

   endRecordingCommandList( cmdList );

   submitCommandList( cmdList );
   presentFrame();

   destroyCommandList( cmdList );

   renderBackendCleanup();
}

void VKSandbox::postLoop() { destroyVertexBuffer( _vertexBuffer ); }

VKSandbox::~VKSandbox() {}
}
