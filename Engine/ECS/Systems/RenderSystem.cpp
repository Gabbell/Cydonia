#include <ECS/Systems/RenderSystem.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>
#include <ECS/SharedComponents/CameraComponent.h>

#include <cmath>

namespace cyd
{
static Rectangle viewport;
static PipelineInfo pipInfo;
static RenderPassInfo renderPassInfo;

static UniformBufferHandle viewBuffer;
static UniformBufferHandle lightBuffer;

// Static function signatures only to keep things tidy and the important stuff at the top
static void preparePipeline();

// Init
// ================================================================================================
bool RenderSystem::init()
{
   preparePipeline();

   // Flipping Y in viewport since we want Y to be up (like GL)
   viewport.offsetX = 0.0f;
   viewport.offsetY = static_cast<float>( pipInfo.extent.height );
   viewport.width   = static_cast<float>( pipInfo.extent.width );
   viewport.height  = -static_cast<float>( pipInfo.extent.height );

   viewBuffer = GRIS::CreateUniformBuffer( sizeof( glm::mat4 ) * 2 );

   // Uploading light information
   lightBuffer = GRIS::CreateUniformBuffer( sizeof( glm::vec4 ) * 3 );

   glm::vec4 lightCol = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );

   GRIS::CopyToUniformBuffer(
       lightBuffer, &lightCol, sizeof( glm::vec4 ) * 2, sizeof( glm::vec4 ) );

   return true;
}

// Tick
// ================================================================================================
void RenderSystem::tick( double deltaS )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();

   static double timeElapsed = 0;
   timeElapsed += deltaS;

   const float x = 100.0f * static_cast<float>( std::cos( timeElapsed ) );
   const float z = 100.0f * static_cast<float>( std::sin( timeElapsed ) );
   glm::vec4 lightPos( x, 0.0f, z, 1.0f );

   GRIS::CopyToUniformBuffer( viewBuffer, &camera.vp, 0, sizeof( camera.vp ) );
   GRIS::CopyToUniformBuffer( lightBuffer, &camera.pos, 0, sizeof( glm::vec4 ) );
   GRIS::CopyToUniformBuffer( lightBuffer, &lightPos, sizeof( glm::vec4 ), sizeof( glm::vec4 ) );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, viewport );

   // Main pass
   GRIS::BeginRenderPassSwapchain( cmdList, renderPassInfo );
   {
      GRIS::BindPipeline( cmdList, pipInfo );

      // Bind view and environment values (Alpha)
      GRIS::BindUniformBuffer( cmdList, viewBuffer, 0, 0 );
      GRIS::BindUniformBuffer( cmdList, lightBuffer, 0, 1 );

      // Bind shader control values (Beta)
      // GRIS::BindUniformBuffer( cmdList, betaBuffer, 1, 0 );

      for( const auto& compPair : m_components )
      {
         // This system is read only
         const TransformComponent& transform   = *std::get<TransformComponent*>( compPair.second );
         const RenderableComponent& renderable = *std::get<RenderableComponent*>( compPair.second );

         glm::mat4 model = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                           glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                           glm::toMat4( transform.rotation );

         // Bind material properties (Gamma)
         GRIS::BindUniformBuffer( cmdList, renderable.matBuffer, 1, 0 );
         GRIS::BindTexture( cmdList, renderable.matTexture, 1, 1 );

         // Update model properties (Epsilon)
         GRIS::UpdateConstantBuffer( cmdList, VERTEX_STAGE, 0, sizeof( glm::mat4 ), &model );

         GRIS::BindVertexBuffer( cmdList, renderable.vertexBuffer );

         if( renderable.indexBuffer != Handle::INVALID_HANDLE )
         {
            GRIS::BindIndexBuffer<uint16_t>( cmdList, renderable.indexBuffer );
            GRIS::DrawVerticesIndexed( cmdList, renderable.indexCount );
         }
         else
         {
            GRIS::DrawVertices( cmdList, 6 );
         }
      }
   }
   GRIS::EndRenderPass( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::WaitOnCommandList( cmdList );

   GRIS::PresentFrame();

   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();
}

void preparePipeline()
{
   // Alpha layout - View and environment (low frequency updates)
   // ==============================================================================================
   DescriptorSetLayoutInfo alphaLayout;

   ShaderResourceInfo viewInfo = {};
   viewInfo.type               = ShaderResourceType::UNIFORM;
   viewInfo.stages             = VERTEX_STAGE;
   viewInfo.binding            = 0;

   ShaderResourceInfo lightInfo = {};
   lightInfo.type               = ShaderResourceType::UNIFORM;
   lightInfo.stages             = FRAGMENT_STAGE;
   lightInfo.binding            = 1;

   alphaLayout.shaderResources.push_back( viewInfo );
   alphaLayout.shaderResources.push_back( lightInfo );

   // Beta layout - Shader control values (medium frequency updates)
   // ==============================================================================================
   // DescriptorSetLayoutInfo betaLayout;

   // ShaderResourceInfo shaderControlInfo = {};
   // shaderControlInfo.type               = ShaderResourceType::UNIFORM;
   // shaderControlInfo.stages             = VERTEX_STAGE | FRAGMENT_STAGE;
   // shaderControlInfo.binding            = 0;

   // betaLayout.shaderResources.push_back( shaderControlInfo );

   // Gamma layout - Material properties (high frequency updates)
   // ==============================================================================================
   DescriptorSetLayoutInfo gammaLayout;

   ShaderResourceInfo matInfo = {};
   matInfo.type               = ShaderResourceType::UNIFORM;
   matInfo.stages             = FRAGMENT_STAGE;
   matInfo.binding            = 0;

   ShaderResourceInfo texInfo = {};
   texInfo.type               = ShaderResourceType::COMBINED_IMAGE_SAMPLER;
   texInfo.stages             = FRAGMENT_STAGE;
   texInfo.binding            = 1;

   gammaLayout.shaderResources.push_back( matInfo );
   gammaLayout.shaderResources.push_back( texInfo );

   // Epsilon layout - Model transforms (very high frequency updates)
   // ==============================================================================================
   PushConstantRange epsilonRange = {};
   epsilonRange.stages            = VERTEX_STAGE;
   epsilonRange.offset            = 0;
   epsilonRange.size              = sizeof( glm::mat4 );

   // Pipeline layout
   // ==============================================================================================
   pipInfo.pipLayout.descSets.push_back( alphaLayout );  // Set 0
   // pipInfo.pipLayout.descSets.push_back( betaLayout );   // Set 1
   pipInfo.pipLayout.descSets.push_back( gammaLayout );  // Set 1
   pipInfo.pipLayout.ranges.push_back( epsilonRange );   // Push constant

   // Pipeline Info
   // ==============================================================================================
   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = {1920, 1080};
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = {"phongTex_vert", "phongTex_frag"};

   // Attachments
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

   renderPassInfo.attachments.push_back( colorPresentation );
   renderPassInfo.attachments.push_back( depthPresentation );
}
}
