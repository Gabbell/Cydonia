#include <Graphics/Scene/Frustum.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <glm/glm.hpp>

#include <algorithm>

namespace CYD
{
BoundingBox Frustum::getWorldAABB() const
{
   BoundingBox worldAABB( m_corners[NEAR_BOTTOM_LEFT], m_corners[FAR_TOP_RIGHT] );
   for( uint32_t i = 0; i < CORNER_COUNT; ++i )
   {
      worldAABB.min.x = std::min( worldAABB.min.x, m_corners[i].x );
      worldAABB.min.y = std::min( worldAABB.min.y, m_corners[i].y );
      worldAABB.min.z = std::min( worldAABB.min.z, m_corners[i].z );

      worldAABB.max.x = std::max( worldAABB.max.x, m_corners[i].x );
      worldAABB.max.y = std::max( worldAABB.max.y, m_corners[i].y );
      worldAABB.max.z = std::max( worldAABB.max.z, m_corners[i].z );
   }

   return worldAABB;
}

void Frustum::update( const glm::mat4& projMat, const glm::mat4& viewMat )
{
   const glm::mat4 MVP        = projMat * viewMat;
   const glm::mat4 inverseMVP = glm::inverse( MVP );

   // Updating planes
   m_planes[LEFT].x = MVP[0].w + MVP[0].x;
   m_planes[LEFT].y = MVP[1].w + MVP[1].x;
   m_planes[LEFT].z = MVP[2].w + MVP[2].x;
   m_planes[LEFT].w = MVP[3].w + MVP[3].x;

   m_planes[RIGHT].x = MVP[0].w - MVP[0].x;
   m_planes[RIGHT].y = MVP[1].w - MVP[1].x;
   m_planes[RIGHT].z = MVP[2].w - MVP[2].x;
   m_planes[RIGHT].w = MVP[3].w - MVP[3].x;

   m_planes[TOP].x = MVP[0].w - MVP[0].y;
   m_planes[TOP].y = MVP[1].w - MVP[1].y;
   m_planes[TOP].z = MVP[2].w - MVP[2].y;
   m_planes[TOP].w = MVP[3].w - MVP[3].y;

   m_planes[BOTTOM].x = MVP[0].w + MVP[0].y;
   m_planes[BOTTOM].y = MVP[1].w + MVP[1].y;
   m_planes[BOTTOM].z = MVP[2].w + MVP[2].y;
   m_planes[BOTTOM].w = MVP[3].w + MVP[3].y;

   m_planes[NEAR].x = MVP[0].w + MVP[0].z;
   m_planes[NEAR].y = MVP[1].w + MVP[1].z;
   m_planes[NEAR].z = MVP[2].w + MVP[2].z;
   m_planes[NEAR].w = MVP[3].w + MVP[3].z;

   m_planes[FAR].x = MVP[0].w - MVP[0].z;
   m_planes[FAR].y = MVP[1].w - MVP[1].z;
   m_planes[FAR].z = MVP[2].w - MVP[2].z;
   m_planes[FAR].w = MVP[3].w - MVP[3].z;

   for( uint32_t i = 0; i < PLANE_COUNT; i++ )
   {
      float length = sqrtf(
          m_planes[i].x * m_planes[i].x + m_planes[i].y * m_planes[i].y +
          m_planes[i].z * m_planes[i].z );
      m_planes[i] /= length;
   }

   // Updating corners
   m_corners[NEAR_BOTTOM_LEFT]  = glm::vec3( -1.0f, -1.0f, 0.0f );
   m_corners[NEAR_TOP_LEFT]     = glm::vec3( -1.0f, 1.0f, 0.0f );
   m_corners[NEAR_TOP_RIGHT]    = glm::vec3( 1.0f, 1.0f, 0.0f );
   m_corners[NEAR_BOTTOM_RIGHT] = glm::vec3( 1.0f, -1.0f, 0.0f );
   m_corners[FAR_BOTTOM_LEFT]   = glm::vec3( -1.0f, -1.0f, 1.0f );
   m_corners[FAR_TOP_LEFT]      = glm::vec3( -1.0f, 1.0f, 1.0f );
   m_corners[FAR_TOP_RIGHT]     = glm::vec3( 1.0f, 1.0f, 1.0f );
   m_corners[FAR_BOTTOM_RIGHT]  = glm::vec3( 1.0f, -1.0f, 1.0f );
   for( uint32_t i = 0; i < CORNER_COUNT; i++ )
   {
      const glm::vec4 worldSpaceCorner = inverseMVP * glm::vec4( m_corners[i], 1.0f );

      m_corners[i] = worldSpaceCorner / worldSpaceCorner.w;
   }
}

void Frustum::draw( CmdListHandle cmdList ) const {}
}
