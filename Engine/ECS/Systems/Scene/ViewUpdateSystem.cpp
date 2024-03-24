#include <ECS/Systems/Scene/ViewUpdateSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/Utility/Transforms.h>
#include <Graphics/Scene/Frustum.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/ViewComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

namespace CYD
{
// ================================================================================================
void ViewUpdateSystem::sort()
{
   auto viewSort = []( const EntityEntry& first, const EntityEntry& second )
   {
      const ViewComponent& firstView  = GetComponent<ViewComponent>( first );
      const ViewComponent& secondView = GetComponent<ViewComponent>( second );

      return firstView.fitToEntity > secondView.fitToEntity;
   };

   std::sort( m_entities.begin(), m_entities.end(), viewSort );
}

// ================================================================================================
void ViewUpdateSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   // Write component
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   CYD_ASSERT( m_entities.size() <= SceneComponent::MAX_VIEWS );

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      ViewComponent& view                 = GetComponent<ViewComponent>( entityEntry );

      // =============================================================================================
      // Assign a new index if the view doesn't have one
      if( view.index == ViewComponent::INVALID_VIEW_INDEX )
      {
         const std::string& entityName = m_ecs->getEntityName( entityEntry.handle );

         // Could not find the view, seeing if there's a free spot
         const auto it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "" );
         if( it == scene.viewNames.end() )
         {
            CYD_ASSERT( !"Something went wrong when trying to find a free view spot" );
            return;
         }

         view.index = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

         scene.viewNames[view.index] = entityName;
      }

      // =============================================================================================
      // Updating view and projection matrix
      const glm::vec3 forward = glm::rotate( transform.rotation, glm::vec3( 0.0f, 0.0, 1.0f ) );

      const bool fitToEntityView = view.fitToEntity != Entity::INVALID_ENTITY;
      if( fitToEntityView )
      {
         const Entity* fitToEntity = m_ecs->getEntity( view.fitToEntity );
         CYD_ASSERT( fitToEntity && "Could not find entity" );

         const ViewComponent* otherView = fitToEntity->getComponent<ViewComponent>();
         CYD_ASSERT( otherView && "Entity did not have a view component" );

         const ViewShaderParams& otherViewParams = scene.views[otherView->index];

         // TODO This should be precalculated
         const Frustum otherViewFrustum =
             Frustum( otherViewParams.projMat, otherViewParams.viewMat );

         glm::vec3 center = glm::vec3( 0.0f, 0.0f, 0.0f );
         for( uint32_t i = 0; i < Frustum::CORNER_COUNT; ++i )
         {
            center += otherViewFrustum.getCorner( static_cast<Frustum::Corner>( i ) );
         }
         center /= Frustum::CORNER_COUNT;

         view.viewMat = glm::lookAtLH( center, center + forward, glm::vec3( 0.0f, 1.0f, 0.0f ) );

         glm::vec3 minValues = glm::vec3( INFINITY, INFINITY, INFINITY );
         glm::vec3 maxValues = glm::vec3( -INFINITY, -INFINITY, -INFINITY );
         for( uint32_t i = 0; i < Frustum::CORNER_COUNT; ++i )
         {
            const glm::vec3& corner =
                otherViewFrustum.getCorner( static_cast<Frustum::Corner>( i ) );

            const glm::vec4 viewSpaceCorner = view.viewMat * glm::vec4( corner, 1.0f );

            minValues = glm::min( minValues, glm::vec3( viewSpaceCorner ) );
            maxValues = glm::max( maxValues, glm::vec3( viewSpaceCorner ) );
         }

         view.projMat = Transform::Ortho(
             minValues.x, maxValues.x, minValues.y, maxValues.y, minValues.z, maxValues.z, true );
      }
      else
      {
         view.viewMat = glm::lookAtLH(
             transform.position, transform.position + forward, glm::vec3( 0.0f, 1.0f, 0.0f ) );

         switch( view.projMode )
         {
            case ViewComponent::ProjectionMode::PERSPECTIVE:
               view.projMat = Transform::Perspective(
                   view.params.perspective.fov,
                   static_cast<float>( scene.extent.width ),
                   static_cast<float>( scene.extent.height ),
                   view.near,
                   view.far,
                   true /*invertY*/,
                   view.reverseZ );
               break;
            case ViewComponent::ProjectionMode::ORTHOGRAPHIC:
               view.projMat = Transform::Ortho(
                   view.params.ortho.left,
                   view.params.ortho.right,
                   view.params.ortho.bottom,
                   view.params.ortho.top,
                   view.near,
                   view.far,
                   true /*invertY*/,
                   view.reverseZ );
               break;
            case ViewComponent::ProjectionMode::UNKNOWN:
               view.projMat = glm::mat4( 1.0f );
               break;
         }
      }

      // =============================================================================================
      // Getting the right view UBOs at this index and updating them
      ViewShaderParams& viewParams               = scene.views[view.index];
      InverseViewShaderParams& inverseViewParams = scene.inverseViews[view.index];
      FrustumShaderParams& frustumParams         = scene.frustums[view.index];

      viewParams.position = glm::vec4( transform.position, 1.0f );

      viewParams.viewMat = view.viewMat;
      viewParams.projMat = view.projMat;

      inverseViewParams.position   = glm::vec4( transform.position, 1.0f );
      inverseViewParams.invViewMat = glm::inverse( viewParams.viewMat );
      inverseViewParams.invProjMat = glm::inverse( viewParams.projMat );

      Frustum viewFrustum( viewParams.projMat, viewParams.viewMat );
      for( uint32_t i = 0; i < Frustum::PLANE_COUNT; ++i )
      {
         frustumParams.planes[i] = viewFrustum.getPlane( static_cast<Frustum::Plane>( i ) );
      }

#if CYD_DEBUG
      if( view.updateDebugFrustum )
      {
         view.debugInverseViewMat = inverseViewParams.invViewMat;
         view.debugInverseProjMat = inverseViewParams.invProjMat;
         view.updateDebugFrustum  = false;
      }
#endif
   }

   // =============================================================================================
   // Transferring all the view data to their respective buffer
   UploadToBufferInfo info;

   info = { 0, sizeof( scene.views ) };
   GRIS::UploadToBuffer( scene.viewsBuffer, &scene.views, info );

   info = { 0, sizeof( scene.inverseViews ) };
   GRIS::UploadToBuffer( scene.inverseViewsBuffer, &scene.inverseViews, info );

   info = { 0, sizeof( scene.frustums ) };
   GRIS::UploadToBuffer( scene.frustumsBuffer, &scene.frustums, info );
}
}
