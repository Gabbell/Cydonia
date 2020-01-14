#include <ECS/Components/RenderableComponent.h>

#include <Graphics/RenderInterface.h>

#include <array>

namespace cyd
{
bool RenderableComponent::init()
{
   // Creating pipeline
   extent.width  = 1280;
   extent.height = 720;

   Attachment colorPresentation;
   colorPresentation.format  = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp  = LoadOp::CLEAR;
   colorPresentation.storeOp = StoreOp::STORE;
   colorPresentation.type    = AttachmentType::COLOR;
   colorPresentation.layout  = ImageLayout::PRESENTATION;

   Attachment depthPresentation;
   depthPresentation.format  = PixelFormat::D32_SFLOAT;
   depthPresentation.loadOp  = LoadOp::CLEAR;
   depthPresentation.storeOp = StoreOp::DONT_CARE;
   depthPresentation.type    = AttachmentType::DEPTH_STENCIL;
   depthPresentation.layout  = ImageLayout::DEPTH_STENCIL;

   RenderPassInfo renderPassInfo;
   renderPassInfo.attachments.push_back( colorPresentation );
   renderPassInfo.attachments.push_back( depthPresentation );

   ShaderObjectInfo uboInfo;
   uboInfo.type    = ShaderObjectType::UNIFORM;
   uboInfo.stages  = VERTEX_STAGE;
   uboInfo.binding = 0;

   ShaderObjectInfo texInfo;
   texInfo.type    = ShaderObjectType::COMBINED_IMAGE_SAMPLER;
   texInfo.stages  = FRAGMENT_STAGE;
   texInfo.binding = 1;

   pipLayoutInfo.descSetLayout.shaderObjects.push_back( uboInfo );
   pipLayoutInfo.descSetLayout.shaderObjects.push_back( texInfo );

   pipInfo.renderPass = renderPassInfo;
   pipInfo.drawPrim   = DrawPrimitive::TRIANGLES;
   pipInfo.extent     = extent;
   pipInfo.polyMode   = PolygonMode::FILL;
   pipInfo.shaders    = { "defaultTex_vert", "defaultTex_frag" };
   pipInfo.pipLayout  = pipLayoutInfo;

   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   uboBuffer = GRIS::CreateUniformBuffer( sizeof( MVP ), 0, pipLayoutInfo.descSetLayout );

   // Triangle
   const std::vector<Vertex> vertices = {
       { { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
       { { 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
       { { 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } } };

   // Placeholder texture
   std::array<uint32_t, 4> texData = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF };
   const size_t texSize            = sizeof( texData[0] ) * texData.size();

   TextureDescription texDesc;
   texDesc.size   = texSize;
   texDesc.width  = 2;
   texDesc.height = 2;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   GRIS::StartRecordingCommandList( transferList );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   texture =
       GRIS::CreateTexture( transferList, texDesc, 1, pipLayoutInfo.descSetLayout, texData.data() );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

void RenderableComponent::uninit()
{
   GRIS::DestroyUniformBuffer( uboBuffer );
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
   GRIS::DestroyTexture( texture );
}

RenderableComponent::~RenderableComponent() = default;
}
