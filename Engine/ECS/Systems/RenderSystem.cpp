#include <ECS/Systems/RenderSystem.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>
#include <ECS/RenderPipelines.h>
#include <ECS/Components/RenderableTypes.h>
#include <ECS/SharedComponents/CameraComponent.h>

#include <cmath>

namespace cyd
{
static Rectangle viewport;
static constexpr uint32_t ENV_VIEW   = 0;  // Environment/View Set
static constexpr uint32_t MAX_LIGHTS = 4;

struct EnvFragmentUBO
{
   glm::vec4 enabled[MAX_LIGHTS];
   glm::vec4 lightPositions[MAX_LIGHTS];
   glm::vec4 lightColors[MAX_LIGHTS];
   glm::vec4 viewPos;
} envFragmentUBO;

void RenderSystem::tick( double deltaS )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();

   static double timeElapsed = 0;
   timeElapsed += deltaS;

   const float x = 30.0f * static_cast<float>( std::cos( timeElapsed ) );
   const float z = 30.0f * static_cast<float>( std::sin( timeElapsed ) );

   envFragmentUBO.lightPositions[0] = glm::vec4( x, 0.0f, z, 1.0f );
   envFragmentUBO.lightPositions[1] = glm::vec4( -x, 0.0f, -z, 1.0f );
   envFragmentUBO.lightPositions[2] = glm::vec4( 0, x, z, 1.0f );
   envFragmentUBO.lightPositions[3] = glm::vec4( 0, -x, -z, 1.0f );

   envFragmentUBO.viewPos = camera.pos;

   GRIS::CopyToUniformBuffer( envVertexBuffer, &camera.vp, 0, sizeof( camera.vp ) );
   GRIS::CopyToUniformBuffer( envFragmentBuffer, &envFragmentUBO, 0, sizeof( EnvFragmentUBO ) );

   GRIS::BindUniformBuffer( cmdList, envVertexBuffer, ENV_VIEW, 0 );
   GRIS::BindUniformBuffer( cmdList, envFragmentBuffer, ENV_VIEW, 1 );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, viewport );

   // Main pass
   GRIS::BeginRenderPassSwapchain( cmdList, true );

   // TODO Either somehow sort renderables by PSO or find a way to decrease calls to BindPipeline
   // as much as possible to reduce overhead.
   for( const auto& compPair : m_components )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );
      const glm::mat4 model = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                              glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                              glm::toMat4( transform.rotation );

      const RenderableComponent* renderable = std::get<RenderableComponent*>( compPair.second );
      switch( renderable->type )
      {
         case RenderableType::PHONG:
            PhongPipeline::render( cmdList, model, renderable );
            break;
         case RenderableType::PBR:
            PBRPipeline::render( cmdList, model, renderable );
            break;
         default:
            CYDASSERT( !"Unknown renderable type" );
      }

      GRIS::BindVertexBuffer( cmdList, renderable->vertexBuffer );

      if( renderable->indexBuffer != Handle::INVALID_HANDLE )
      {
         // This renderable has an index buffer, use it to draw
         GRIS::BindIndexBuffer<uint32_t>( cmdList, renderable->indexBuffer );
         GRIS::DrawVerticesIndexed( cmdList, renderable->indexCount );
      }
      else
      {
         GRIS::DrawVertices( cmdList, renderable->vertexCount );
      }
   }

   GRIS::EndRenderPass( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );

   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();
}

bool RenderSystem::init()
{
   // Flipping Y in viewport since we want Y to be up (like GL)
   viewport.offsetX = 0.0f;
   viewport.offsetY = 1080;
   viewport.width   = 1920;
   viewport.height  = -1080;

   envVertexBuffer   = GRIS::CreateUniformBuffer( sizeof( glm::mat4 ) * 2 );
   envFragmentBuffer = GRIS::CreateUniformBuffer( sizeof( EnvFragmentUBO ) );

   envFragmentUBO.enabled[0] = glm::vec4( true, false, false, false );
   envFragmentUBO.enabled[1] = glm::vec4( true, false, false, false );
   envFragmentUBO.enabled[2] = glm::vec4( true, false, false, false );
   envFragmentUBO.enabled[3] = glm::vec4( true, false, false, false );

   envFragmentUBO.lightColors[0] = glm::vec4( 255.0f, 0.0f, 0.0f, 255.0f );
   envFragmentUBO.lightColors[1] = glm::vec4( 0.0f, 255.0f, 0.0f, 255.0f );
   envFragmentUBO.lightColors[2] = glm::vec4( 0.0f, 0.0f, 255.0f, 255.0f );
   envFragmentUBO.lightColors[3] = glm::vec4( 0.0f, 255.0f, 255.0f, 255.0f );

   return true;
}

void RenderSystem::uninit()
{
   GRIS::DestroyUniformBuffer( envFragmentBuffer );
   GRIS::DestroyUniformBuffer( envVertexBuffer );
}
}
