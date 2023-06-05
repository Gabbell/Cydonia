#include <ECS/Systems/Scene/CameraSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace CYD
{
void CameraSystem::tick( double /*deltaS*/ )
{
   // Write component
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   CYDASSERT( m_entities.size() <= SceneComponent::MAX_VIEWS );

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
            CYDASSERT( !"Something went wrong when trying to find a free view UBO spot" );
            return;
         }
      }

      const uint32_t uboIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

      // Naming this view
      scene.viewNames[uboIdx] = camera.viewName;

      // Getting the right view UBO at this index and updating it
      SceneComponent::ViewUBO& view = scene.views[uboIdx];

      view.position = glm::vec4( transform.position, 1.0f );

      view.viewMat = glm::toMat4( glm::conjugate( transform.rotation ) ) *
                     glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / transform.scaling ) *
                     glm::translate( glm::mat4( 1.0f ), -transform.position );

      switch( camera.projMode )
      {
         case CameraComponent::ProjectionMode::PERSPECTIVE:
            view.projMat = glm::perspectiveZO(
                glm::radians( camera.fov ), camera.aspectRatio, camera.near, camera.far );
            break;
         case CameraComponent::ProjectionMode::ORTHOGRAPHIC:
            view.projMat = glm::orthoZO(
                camera.left, camera.right, camera.bottom, camera.top, camera.near, camera.far );
            break;
         case CameraComponent::ProjectionMode::UNKNOWN:
            view.projMat = glm::mat4( 1.0f );
            break;
      }
   }

   // Transferring all the views to one buffer
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER, "CameraSystem" );

   GRIS::StartRecordingCommandList( transferList );

   GRIS::CopyToBuffer( scene.viewsBuffer, &scene.views, 0, sizeof( scene.views ) );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}
}
