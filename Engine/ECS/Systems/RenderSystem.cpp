#include <ECS/Systems/RenderSystem.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>
#include <ECS/SharedComponents/CameraComponent.h>

#include <cmath>

namespace cyd
{
static Rectangle viewport;
static PipelineInfo pipInfo;

static UniformBufferHandle alphaVertBuffer;
static UniformBufferHandle alphaFragBuffer;

// Static function signatures only to keep things tidy and the important stuff at the top
static void preparePipeline();

static constexpr char VERTEX_SHADER[]   = "pbrTex_vert";
static constexpr char FRAGMENT_SHADER[] = "pbrTex_frag";
static constexpr uint32_t MAX_LIGHTS    = 2;

enum DescriptorSet
{
   ALPHA = 0,  // View and environment values (low frequency updates)
   GAMMA = 1   // Material properties (high frequency updates)
};

static struct AlphaFragUBO
{
   glm::vec4 enabled[MAX_LIGHTS];
   glm::vec4 lightPositions[MAX_LIGHTS];
   glm::vec4 lightColors[MAX_LIGHTS];
   glm::vec4 viewPos;
} alphaFragUBO;

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

   alphaVertBuffer = GRIS::CreateUniformBuffer( sizeof( glm::mat4 ) * 2 );
   alphaFragBuffer = GRIS::CreateUniformBuffer( sizeof( AlphaFragUBO ) );

   alphaFragUBO.enabled[0] = glm::vec4( true, false, false, false );
   alphaFragUBO.enabled[1] = glm::vec4( true, false, false, false );

   alphaFragUBO.lightColors[0] = glm::vec4( 255.0f, 0.0f, 0.0f, 1.0f );
   alphaFragUBO.lightColors[1] = glm::vec4( 0.0f, 255.0f, 255.0f, 1.0f );

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

   const float x = 22.0f * static_cast<float>( std::cos( timeElapsed ) );
   const float z = 22.0f * static_cast<float>( std::sin( timeElapsed ) );

   alphaFragUBO.lightPositions[0] = glm::vec4( x, 0.0f, z, 1.0f );
   alphaFragUBO.lightPositions[1] = glm::vec4( -x, 0.0f, -z, 1.0f );

   alphaFragUBO.viewPos = camera.pos;

   GRIS::CopyToUniformBuffer( alphaVertBuffer, &camera.vp, 0, sizeof( camera.vp ) );
   GRIS::CopyToUniformBuffer( alphaFragBuffer, &alphaFragUBO, 0, sizeof( AlphaFragUBO ) );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, viewport );

   // Main pass
   GRIS::BeginRenderPassSwapchain( cmdList, true );
   {
      GRIS::BindPipeline( cmdList, pipInfo );

      GRIS::BindUniformBuffer( cmdList, alphaVertBuffer, ALPHA, 0 );
      GRIS::BindUniformBuffer( cmdList, alphaFragBuffer, ALPHA, 1 );

      for( const auto& compPair : m_components )
      {
         // This system is read only
         const TransformComponent& transform   = *std::get<TransformComponent*>( compPair.second );
         const RenderableComponent& renderable = *std::get<RenderableComponent*>( compPair.second );

         glm::mat4 model = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                           glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                           glm::toMat4( transform.rotation );

         GRIS::BindTexture( cmdList, renderable.albedo, GAMMA, 0 );
         GRIS::BindTexture( cmdList, renderable.normalMap, GAMMA, 1 );
         GRIS::BindTexture( cmdList, renderable.metallicMap, GAMMA, 2 );
         GRIS::BindTexture( cmdList, renderable.roughnessMap, GAMMA, 3 );
         GRIS::BindTexture( cmdList, renderable.ambientOcclusionMap, GAMMA, 4 );
         GRIS::BindTexture( cmdList, renderable.heightMap, GAMMA, 5 );

         // Update model properties
         GRIS::UpdateConstantBuffer( cmdList, VERTEX_STAGE, 0, sizeof( glm::mat4 ), &model );

         GRIS::BindVertexBuffer( cmdList, renderable.vertexBuffer );

         if( renderable.indexBuffer != Handle::INVALID_HANDLE )
         {
            // This renderable has an index buffer, use it to draw
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
   // Alpha layout
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

   // Gamma layout
   // ==============================================================================================
   DescriptorSetLayoutInfo gammaLayout;

   ShaderResourceInfo texInfo = {};
   texInfo.type               = ShaderResourceType::COMBINED_IMAGE_SAMPLER;
   texInfo.stages             = FRAGMENT_STAGE;

   // Albedo, normal, metallic, roughness, ambient occlusion
   for( uint32_t i = 0; i < 5; ++i )
   {
      texInfo.binding = i;
      gammaLayout.shaderResources.push_back( texInfo );
   }

   texInfo.binding = 5;
   texInfo.stages  = VERTEX_STAGE;
   gammaLayout.shaderResources.push_back( texInfo );  // Heightmap

   // Epsilon layout
   // ==============================================================================================
   PushConstantRange modelRange = {};
   modelRange.stages            = VERTEX_STAGE;
   modelRange.offset            = 0;
   modelRange.size              = sizeof( glm::mat4 );

   // Pipeline layout
   // ==============================================================================================
   // Sets
   pipInfo.pipLayout.descSets.push_back( alphaLayout );  // Set 0
   pipInfo.pipLayout.descSets.push_back( gammaLayout );  // Set 1

   // Push constants
   pipInfo.pipLayout.ranges.push_back( modelRange );

   // Pipeline Info
   // ==============================================================================================
   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };

   // Specialization Constants
   pipInfo.constants.add( FRAGMENT_SHADER, 0, MAX_LIGHTS );
}
}
