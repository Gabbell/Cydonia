#include <ECS/Systems/RenderSystem.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>
#include <ECS/SharedComponents/CameraComponent.h>

namespace cyd
{

static PipelineInfo pipInfo;
static RenderPassInfo renderPassInfo;
static UniformBufferHandle alphaBuffer;

// Static function signatures only to keep things tidy and the important stuff at the top
static void preparePipeline();

// Init
// ================================================================================================
bool RenderSystem::init()
{
   alphaBuffer = GRIS::CreateUniformBuffer( sizeof( glm::mat4 ) * 2 );

   preparePipeline();
   return true;
}

// Tick
// ================================================================================================
void RenderSystem::tick( double /*deltaS*/ )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
   GRIS::CopyToUniformBuffer( alphaBuffer, &camera.vp );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, { { 0.0f, 0.0f }, pipInfo.extent } );

   // Main pass
   GRIS::BeginRenderPassSwapchain( cmdList, renderPassInfo );
   {
      GRIS::BindPipeline( cmdList, pipInfo );

      // Bind view and environment values (Alpha)
      GRIS::BindUniformBuffer( cmdList, alphaBuffer, 0, 0 );

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
         GRIS::BindUniformBuffer( cmdList, renderable.matBuffer, 2, 0 );
         GRIS::BindTexture( cmdList, renderable.matTexture, 2, 1 );

         // Update model properties (Epsilon)
         GRIS::UpdateConstantBuffer( cmdList, VERTEX_STAGE, 0, sizeof( glm::mat4 ), &model );

         // GRIS::BindIndexBuffer( cmdList, renderable.indexBuffer );
         GRIS::BindVertexBuffer( cmdList, renderable.vertexBuffer );
         GRIS::DrawVertices( cmdList, 36 );  // TODO Generalize
      }
   }
   GRIS::EndRenderPass( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
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
   viewInfo.stages             = VERTEX_STAGE | FRAGMENT_STAGE;
   viewInfo.binding            = 0;

   alphaLayout.shaderResources.push_back( viewInfo );

   // Beta layout - Shader control values (medium frequency updates)
   // ==============================================================================================
   DescriptorSetLayoutInfo betaLayout;

   ShaderResourceInfo shaderControlInfo = {};
   shaderControlInfo.type               = ShaderResourceType::UNIFORM;
   shaderControlInfo.stages             = VERTEX_STAGE | FRAGMENT_STAGE;
   shaderControlInfo.binding            = 0;

   betaLayout.shaderResources.push_back( shaderControlInfo );

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
   pipInfo.pipLayout.descSets.push_back( betaLayout );   // Set 1
   pipInfo.pipLayout.descSets.push_back( gammaLayout );  // Set 2
   pipInfo.pipLayout.ranges.push_back( epsilonRange );   // Push constant

   // Pipeline Info
   // ==============================================================================================
   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1280, 720 };  // Current resolution
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { "defaultTex_vert", "defaultTex_frag" };

   // Attachments
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.layout     = ImageLayout::PRESENTATION;

   renderPassInfo.attachments.push_back( colorPresentation );
}
}
