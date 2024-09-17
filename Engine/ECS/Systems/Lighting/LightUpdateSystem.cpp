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

   static double timeElapsed = 0;

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   CYD_ASSERT_AND_RETURN(
       m_entities.size() <= SceneComponent::MAX_LIGHTS && "Too many lights in the scene", return; );

   for( const auto& entityEntry : m_entities )
   {
      TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      LightComponent& light         = GetComponent<LightComponent>( entityEntry );

      // Assign a new index if the component doesn't have one
      if( light.index == LightComponent::INVALID_LIGHT_INDEX )
      {
         const std::string& entityName = m_ecs->getEntityName( entityEntry.handle );

         const auto it = std::find( scene.lightNames.begin(), scene.lightNames.end(), "" );
         if( it == scene.lightNames.end() )
         {
            CYD_ASSERT( !"Something went wrong when trying to find a free light spot" );
            return;
         }

         light.index = static_cast<uint32_t>( std::distance( scene.lightNames.begin(), it ) );

         scene.lightNames[light.index] = entityName;
      }

      if( light.type == LightComponent::Type::DIRECTIONAL )
      {
#if 0
         light.direction = glm::normalize( glm::vec3(
             0.0f,
             20.0f * std::sin( 0.025f * timeElapsed ) - 10.0f,
             -20.0f * std::cos( 0.025f * timeElapsed ) ) );
#endif

         transform.rotation = glm::quatLookAtLH( light.direction, glm::vec3( 0.0, 1.0, 0.0 ) );
      }

      // Updating scene
      LightShaderParams& lightParams = scene.lights[light.index];

      lightParams.position  = glm::vec4( transform.position, 1.0f );
      lightParams.color     = light.color;
      lightParams.direction = glm::vec4( light.direction, 0.0f );
      lightParams.enabled   = glm::vec4( light.enabled, false, false, false );
   }

   // Updating UBOs
   const UploadToBufferInfo info = { 0, sizeof( LightShaderParams ) * m_entities.size() };

   GRIS::UploadToBuffer( scene.lightsBuffer, &scene.lights, info );

   timeElapsed += deltaS;
}
}