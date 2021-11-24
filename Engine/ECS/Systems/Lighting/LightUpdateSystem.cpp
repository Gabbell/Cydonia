#include <ECS/Systems/Lighting/LightUpdateSystem.h>

#include <Graphics/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
void LightUpdateSystem::tick( double deltaS )
{
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   static double timeElapsed = 0;

   for( const auto& entityEntry : m_entities )
   {
      const LightComponent& light   = *std::get<LightComponent*>( entityEntry.arch );
      TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );

      transform.position = glm::vec3(
          25.0f * std::cos( 0.50f * timeElapsed ), 10.0f, 25.0f * std::sin( 0.50f * timeElapsed ) );

      const glm::vec3 direction = glm::normalize( -transform.position );

      transform.rotation = glm::quatLookAt( direction, glm::vec3( 0.0f, 1.0f, 0.0f ) );

      scene.light.position = glm::vec4( transform.position, 1.0f );
      scene.light.color    = light.color;
      scene.light.enabled  = glm::vec4( true, false, false, false );
   }

   timeElapsed += deltaS;

   // Updating UBOs
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER, "LightUpdateSystem" );

   GRIS::StartRecordingCommandList( transferList );

   GRIS::CopyToBuffer( scene.lightsBuffer, &scene.light, 0, sizeof( SceneComponent::LightUBO ) );

   GRIS::EndRecordingCommandList( transferList );

   RenderGraph::AddPass( transferList );
}
}