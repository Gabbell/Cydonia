#include <ECS/Systems/Lighting/LightUpdateSystem.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/Transforms.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
void LightUpdateSystem::tick( double deltaS )
{
   CYD_TRACE();

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   CYD_ASSERT_AND_RETURN(
       m_entities.size() <= SceneComponent::MAX_LIGHTS && "Too many lights in the scene", return; );

   uint32_t lightIdx = 0;
   for( const auto& entityEntry : m_entities )
   {
      const LightComponent& light   = GetComponent<LightComponent>( entityEntry );
      TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );

      if( light.type == LightComponent::Type::DIRECTIONAL )
      {
         // We might not want to do this for every directional light. This is really only
         // relevant for the sun
         transform.position =
             transform.position - ( light.direction * LightComponent::DIRECTIONAL_POSITION_SCALE );

         transform.rotation = glm::quatLookAt( light.direction, glm::vec3( 0.0, 1.0, 0.0 ) );
      }

      // Updating scene
      scene.lights[lightIdx].position  = glm::vec4( transform.position, 1.0f );
      scene.lights[lightIdx].color     = light.color;
      scene.lights[lightIdx].direction = glm::vec4( light.direction, 0.0f );
      scene.lights[lightIdx].enabled   = glm::vec4( light.enabled, false, false, false );

      lightIdx++;
   }

   // Updating UBOs
   const UploadToBufferInfo info = {
       0, sizeof( SceneComponent::LightShaderParams ) * m_entities.size() };
   GRIS::UploadToBuffer( scene.lightsBuffer, &scene.lights, info );
}
}