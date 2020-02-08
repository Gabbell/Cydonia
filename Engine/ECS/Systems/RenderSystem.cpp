#include <ECS/Systems/RenderSystem.h>

#include <Graphics/RenderInterface.h>

namespace cyd
{
struct MVP
{
   glm::mat4 model = glm::mat4( 1.0f );
   glm::mat4 view  = glm::mat4( 1.0f );
   glm::mat4 proj  = glm::mat4( 1.0f );
};

static PipelineInfo pipInfo;
static RenderPassInfo renderPassInfo;

bool RenderSystem::init()
{
   _preparePipeline();
   return true;
}

void RenderSystem::tick( double /*deltaS*/ )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, { { 0.0f, 0.0f }, pipInfo.extent } );

   // First pass: muh triangles
   GRIS::BeginRenderPass( cmdList, renderPassInfo );
   {
      GRIS::BindPipeline( cmdList, pipInfo );

      // Bind view and environment values (Alpha)
      // GRIS::BindUniformBuffer( cmdList, alphaBuffer, 0, 0 );

      // Bind shader control values (Beta)
      // GRIS::BindUniformBuffer( cmdList, betaBuffer, 1, 0 );

      // Render scene
      for( const auto& archPair : m_archetypes )
      {
         // This system is read only
         const TransformComponent& transform   = *std::get<TransformComponent*>( archPair.second );
         const RenderableComponent& renderable = *std::get<RenderableComponent*>( archPair.second );

         glm::mat4 model = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                           glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                           glm::toMat4( transform.rotation );

         // Bind material properties (Gamma)
         GRIS::BindUniformBuffer( cmdList, renderable.matBuffer, 2, 0 );
         GRIS::BindTexture( cmdList, renderable.matTexture, 2, 1 );

         // Update model properties (Epsilon)
         GRIS::UpdateConstantBuffer( cmdList, VERTEX_STAGE, 0, sizeof( MVP::model ), &model );

         // GRIS::BindIndexBuffer( cmdList, renderable.indexBuffer );
         GRIS::BindVertexBuffer( cmdList, renderable.vertexBuffer );
         GRIS::DrawVertices( cmdList, 3 );  // TODO Generalize
      }
   }
   GRIS::EndRenderPass( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::PresentFrame();

   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();
}

void RenderSystem::_preparePipeline() const
{
   // Alpha layout - View and environment (low frequency updates)
   // ==============================================================================================
   DescriptorSetLayoutInfo alphaLayout;

   ShaderResourceInfo viewInfo = {};
   viewInfo.type               = ShaderResourceType::UNIFORM;
   viewInfo.stages             = VERTEX_STAGE;
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
   epsilonRange.size              = sizeof( MVP::model );

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
