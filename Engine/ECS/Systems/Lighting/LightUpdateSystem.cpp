#include <ECS/Systems/Lighting/LightUpdateSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
void LightUpdateSystem::tick( double deltaS )
{
   SceneComponent& scene = ECS::GetSharedComponent<SceneComponent>();

   static double timeElapsed = 0;
   timeElapsed += deltaS;

   const float y = 30.0f * static_cast<float>( std::cos( timeElapsed ) );
   const float z = 30.0f * static_cast<float>( std::sin( timeElapsed ) );

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const LightComponent& light         = *std::get<LightComponent*>( entityEntry.arch );

      scene.dirLight.enabled   = glm::vec4( true, false, false, false );
      scene.dirLight.direction = transform.rotation * glm::vec4( 0.0f, y, z, 1.0f );
      scene.dirLight.color     = light.color;
   }

   // Creating the light UBOs if they don't exist
   if( !scene.lightsBuffer )
   {
      scene.lightsBuffer =
          GRIS::CreateUniformBuffer( sizeof( SceneComponent::DirectionalLightUBO ) );
   }

   // Updating UBOs
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   GRIS::CopyToBuffer(
       scene.lightsBuffer, &scene.dirLight, 0, sizeof( SceneComponent::DirectionalLightUBO ) );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}
}
