#include <ECS/Systems/Lighting/ShadowMapUpdateSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Scene/BoudingBox.h>
#include <Graphics/Scene/Frustum.h>
#include <Graphics/Utility/ShadowMapping.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

#include <glm/gtc/matrix_transform.hpp>

namespace CYD
{
// Creates a light clip-space axis-aligned box
static BoundingBox CreateCascadeAABB( const Frustum& cascadeFrustum, const glm::mat4 lightViewProj )
{
   glm::vec3 minPoint( INFINITY );
   glm::vec3 maxPoint( -INFINITY );
   for( uint32_t i = 0; i < Frustum::CORNER_COUNT; ++i )
   {
      glm::vec4 lightClipSpacePoint =
          lightViewProj *
          glm::vec4( cascadeFrustum.getCorner( static_cast<Frustum::Corner>( i ) ), 1.0f );
      lightClipSpacePoint /= lightClipSpacePoint.w;

      minPoint = glm::min( minPoint, glm::vec3( lightClipSpacePoint ) );
      maxPoint = glm::max( maxPoint, glm::vec3( lightClipSpacePoint ) );
   }

   return BoundingBox( minPoint, maxPoint );
}

static void UpdateTextures( ShadowMapComponent& shadowMap )
{
   if( !shadowMap.depth )
   {
      shadowMap.depth = GRIS::CreateTexture( ShadowMapping::GetDepthTextureDescription(
          shadowMap.resolution, shadowMap.numCascades ) );
   }

   if( !shadowMap.filterable && shadowMap.isFilterable )
   {
      shadowMap.filterable = GRIS::CreateTexture( ShadowMapping::GetFilterableTextureDescription(
          shadowMap.resolution, shadowMap.numCascades ) );
   }
}

static void UpdateFramebuffers( const ShadowMapComponent& shadowMap, SceneComponent& scene )
{
   Framebuffer& shadowmapFB = scene.shadowMapFBs[shadowMap.index];
   if( !shadowmapFB.isValid() )
   {
      ClearValue colorClear;
      colorClear.color.f32[0] = 1.0f;
      colorClear.color.f32[1] = 0.0f;
      colorClear.color.f32[2] = 0.0f;
      colorClear.color.f32[3] = 0.0f;

      ClearValue depthClear;
      depthClear.depthStencil.depth   = 1.0f;
      depthClear.depthStencil.stencil = 0;

      shadowmapFB.resize( shadowMap.resolution, shadowMap.resolution );
      shadowmapFB.setClearAll( true );

      if( shadowMap.filterable )
      {
         shadowmapFB.attach(
             Framebuffer::Index::COLOR,
             shadowMap.filterable,
             Access::FRAGMENT_SHADER_READ,
             colorClear );
         shadowmapFB.attach(
             Framebuffer::Index::DEPTH,
             shadowMap.depth,
             Access::DEPTH_STENCIL_ATTACHMENT_READ,
             depthClear );

         scene.shadowMapTextures[shadowMap.index] = shadowMap.filterable;
      }
      else
      {
         shadowmapFB.attach(
             Framebuffer::Index::COLOR, shadowMap.depth, Access::FRAGMENT_SHADER_READ, depthClear );

         scene.shadowMapTextures[shadowMap.index] = shadowMap.depth;
      }
   }
}

void ShadowMapUpdateSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   for( const auto& entityEntry : m_entities )
   {
      const ViewComponent& view     = GetComponent<ViewComponent>( entityEntry );
      ShadowMapComponent& shadowMap = GetComponent<ShadowMapComponent>( entityEntry );

      // Assign a new index if the component doesn't have one
      if( shadowMap.index == ShadowMapComponent::INVALID_SHADOWMAP_INDEX )
      {
         const std::string& entityName = m_ecs->getEntityName( entityEntry.handle );

         const auto it = std::find( scene.shadowMapNames.begin(), scene.shadowMapNames.end(), "" );
         if( it == scene.shadowMapNames.end() )
         {
            CYD_ASSERT( !"Something went wrong when trying to find a free shadow map spot" );
            return;
         }

         shadowMap.index =
             static_cast<uint32_t>( std::distance( scene.shadowMapNames.begin(), it ) );

         scene.shadowMapNames[shadowMap.index] = entityName;
      }

      const ViewShaderParams& mainViewParams = scene.views[0];
      const ViewShaderParams& viewParams     = scene.views[view.index];

      UpdateTextures( shadowMap );
      UpdateFramebuffers( shadowMap, scene );

      // =========================================================================================
      // Creating crop matrices
      ShadowMapShaderParams& shadowMapParams = scene.shadowMaps[shadowMap.index];

      shadowMapParams.enabled = shadowMap.enabled;

      const bool isCascaded = shadowMap.numCascades > 1;
      if( isCascaded )
      {
         // Partition main view frustum into cascades
         const glm::mat4 lightViewProj = viewParams.projMat * viewParams.viewMat;

         // TODO Get from main camera
         float near = 0.1f;
         float far  = 32000.0f;

         // From parallel-split algorithm (practical split scheme)
         const float lambda = shadowMap.splitLambda;

         float cascadeNear = near;
         for( uint32_t cascade = 0; cascade < shadowMap.numCascades; ++cascade )
         {
            // Getting far plane for this cascade
            const float cascadeRatio =
                static_cast<float>( cascade + 1 ) / ShadowMapping::MAX_CASCADES;
            const float logSplit     = near * pow( far / near, cascadeRatio );
            const float uniformSplit = near + ( far - near ) * cascadeRatio;

            const float practicalSplit = lambda * logSplit + ( 1.0f - lambda ) * uniformSplit;

            const float cascadeFar = practicalSplit;

            // Creating new cascade projection matrix
            // This assumes that the main view is a perspective matrix created with perspectiveLH_ZO
            glm::mat4 cascadeProjMat( 1.0f );
            cascadeProjMat       = mainViewParams.projMat;
            cascadeProjMat[2][2] = cascadeFar / ( cascadeFar - cascadeNear );
            cascadeProjMat[3][2] = -( cascadeFar * cascadeNear ) / ( cascadeFar - cascadeNear );

            cascadeNear = cascadeFar;

            // Create crop matrix
            const Frustum cascadeFrustum( cascadeProjMat, mainViewParams.viewMat );
            BoundingBox cropBB = CreateCascadeAABB( cascadeFrustum, lightViewProj );

            // cropBB.min.z        = 0.0f;
            const float scaleX  = 2.0f / ( cropBB.max.x - cropBB.min.x );
            const float scaleY  = 2.0f / ( cropBB.max.y - cropBB.min.y );
            const float offsetX = -0.5f * ( cropBB.max.x + cropBB.min.x ) * scaleX;
            const float offsetY = -0.5f * ( cropBB.max.y + cropBB.min.y ) * scaleY;
            const float scaleZ  = 1.0f / ( cropBB.max.z - cropBB.min.z );
            const float offsetZ = -cropBB.min.z * scaleZ;

            const float frustumRadius = glm::distance(
                cascadeFrustum.getCorner( Frustum::NEAR_BOTTOM_LEFT ),
                cascadeFrustum.getCorner( Frustum::FAR_TOP_RIGHT ) );

            // Creating a crop matrix that will be applied to the light's projection matrix
            // to only include the cascade's area of interest
            glm::mat4 cropMatrix( 1.0f );
            cropMatrix = glm::translate( cropMatrix, glm::vec3( offsetX, offsetY, offsetZ ) );
            cropMatrix = glm::scale( cropMatrix, glm::vec3( scaleX, scaleY, scaleZ ) );

            shadowMapParams.cascade[cascade].farDepth           = cascadeFar;
            shadowMapParams.cascade[cascade].radius             = frustumRadius;
            shadowMapParams.cascade[cascade].texelSize          = 1.0f / shadowMap.resolution;
            shadowMapParams.cascade[cascade].worldToLightMatrix = cropMatrix * lightViewProj;

            for( uint32_t i = 0; i < Frustum::PLANE_COUNT; ++i )
            {
               shadowMapParams.cascade[cascade].planes[i] =
                   cascadeFrustum.getPlane( static_cast<Frustum::Plane>( i ) );
            }
         }
      }

      UploadToBufferInfo info = { 0, sizeof( scene.shadowMaps ) };
      GRIS::UploadToBuffer( scene.shadowMapsBuffer, &scene.shadowMaps, info );
   }
}
}