#include <ECS/Systems/Scene/CameraSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/Utility/Transforms.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace CYD
{
void CameraSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "CameraSystem" );

   // Write component
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   CYD_ASSERT( m_entities.size() <= SceneComponent::MAX_VIEWS );

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const CameraComponent& camera       = *std::get<CameraComponent*>( entityEntry.arch );

      // Finding view in the scene
      auto it = std::find( scene.viewNames.begin(), scene.viewNames.end(), camera.viewName );
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
      scene.viewNames[viewIdx] = camera.viewName;

      // Getting the right view UBO at this index and updating it
      SceneComponent::ViewShaderParams& view = scene.views[viewIdx];

      view.position = glm::vec4( transform.position, 1.0f );

      const glm::vec3 viewDir = glm::vec4( 0.0f, 0.0f, -1.0f, 1.0f ) *
                                glm::toMat4( glm::conjugate( transform.rotation ) );

      view.viewMat = glm::lookAt(
          transform.position, transform.position + viewDir, glm::vec3( 0.0f, 1.0f, 0.0f ) );

      switch( camera.projMode )
      {
         case CameraComponent::ProjectionMode::PERSPECTIVE:
            view.projMat =
                Transform::Perspective( camera.fov, camera.aspectRatio, camera.near, camera.far );
            break;
         case CameraComponent::ProjectionMode::ORTHOGRAPHIC:
            view.projMat = Transform::Ortho(
                camera.left, camera.right, camera.bottom, camera.top, camera.near, camera.far );
            break;
         case CameraComponent::ProjectionMode::UNKNOWN:
            view.projMat = glm::mat4( 1.0f );
            break;
      }
   }

   // Transferring all the views to one buffer
   GRIS::CopyToBuffer( scene.viewsBuffer, &scene.views, 0, sizeof( scene.views ) );
}
}
