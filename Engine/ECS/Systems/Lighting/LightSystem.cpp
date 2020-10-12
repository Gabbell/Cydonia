#include <ECS/Systems/Lighting/LightSystem.h>

#include <ECS/EntityManager.h>

#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
void LightSystem::tick( double deltaS )
{
   SceneComponent& scene = ECS::GetSharedComponent<SceneComponent>();

   static double timeElapsed = 0;
   timeElapsed += deltaS;

   const float y = 30.0f * static_cast<float>( std::cos( timeElapsed ) );
   const float z = 30.0f * static_cast<float>( std::sin( timeElapsed ) );

   for( const auto& compPair : m_components )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );
      const LightComponent& light         = *std::get<LightComponent*>( compPair.second );

      scene.dirLight.enabled   = glm::vec4( true, false, false, false );
      scene.dirLight.direction = transform.rotation * glm::vec4( 0.0f, y, z, 1.0f );
      scene.dirLight.color     = light.color;
   }
}
}