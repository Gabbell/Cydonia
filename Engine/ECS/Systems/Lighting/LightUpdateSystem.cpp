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
   CYD_TRACE( "LightUpdateSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   static double timeElapsed = 0;

   CYD_ASSERT_AND_RETURN(
       m_entities.size() <= SceneComponent::MAX_LIGHTS && "Too many lights in the scene", return; );

   uint32_t lightIdx = 0;
   for( const auto& entityEntry : m_entities )
   {
      const LightComponent& light   = *std::get<LightComponent*>( entityEntry.arch );
      TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );

      // Updating light transform
      transform.position = glm::vec3(
          25.0f * std::cos( 0.50f * timeElapsed ),
          transform.position.y,
          25.0f * std::sin( 0.50f * timeElapsed ) );

      // Finding view in the scene
      const std::string viewName = "LightView " + std::to_string( lightIdx );
      auto it = std::find( scene.viewNames.begin(), scene.viewNames.end(), viewName );
      if( it == scene.viewNames.end() )
      {
         // Could not find the view, seeing if there's a free spot
         it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "" );
         if( it == scene.viewNames.end() )
         {
            CYD_ASSERT( !"Something went wrong when trying to find a free view UBO spot" );
            return;
         }
      }

      const uint32_t viewIdx =
          static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

      // Naming this view
      scene.viewNames[viewIdx] = viewName;

      // Updating view
      SceneComponent::ViewShaderParams& view = scene.views[viewIdx];

      view.position = glm::vec4( transform.position, 1.0f );

      view.viewMat = glm::lookAt(
          transform.position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f ) );

      view.projMat = Transform::OrthoReverseZ( -71.0f, 71.0f, -71.0f, 71.0f, -100.0f, 100.0f );

      // Updating scene
      scene.lights[lightIdx].position = glm::vec4( transform.position, 1.0f );
      scene.lights[lightIdx].direction =
          -glm::vec4( view.viewMat[0][2], view.viewMat[1][2], view.viewMat[2][2], 0.0f );
      scene.lights[lightIdx].color   = light.color;
      scene.lights[lightIdx].enabled = glm::vec4( light.enabled, false, false, false );

      lightIdx++;
   }

   timeElapsed += deltaS;

   // Updating UBOs
   const size_t sizeToUpdate = sizeof( SceneComponent::LightShaderParams ) * m_entities.size();
   GRIS::CopyToBuffer( scene.lightsBuffer, &scene.lights, 0, sizeToUpdate );
}
}