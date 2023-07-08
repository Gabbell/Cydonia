#pragma once

#include <glm/glm.hpp>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class Frustum
{
  public:
   Frustum()  = default;
   ~Frustum() = default;

   enum Plane
   {
      LEFT,
      RIGHT,
      BOTTOM,
      TOP,
      NEAR,
      FAR,
      COUNT
   };

   void getPlanes( glm::vec4* destination ) const
   {
      memcpy( destination, &m_planes, sizeof( m_planes ) );
   }

   void update( const glm::mat4& projMat, const glm::mat4& viewMat )
   {
      const glm::mat4 mvp = projMat * viewMat;
      m_planes[LEFT].x    = mvp[0].w + mvp[0].x;
      m_planes[LEFT].y    = mvp[1].w + mvp[1].x;
      m_planes[LEFT].z    = mvp[2].w + mvp[2].x;
      m_planes[LEFT].w    = mvp[3].w + mvp[3].x;

      m_planes[RIGHT].x = mvp[0].w - mvp[0].x;
      m_planes[RIGHT].y = mvp[1].w - mvp[1].x;
      m_planes[RIGHT].z = mvp[2].w - mvp[2].x;
      m_planes[RIGHT].w = mvp[3].w - mvp[3].x;

      m_planes[TOP].x = mvp[0].w - mvp[0].y;
      m_planes[TOP].y = mvp[1].w - mvp[1].y;
      m_planes[TOP].z = mvp[2].w - mvp[2].y;
      m_planes[TOP].w = mvp[3].w - mvp[3].y;

      m_planes[BOTTOM].x = mvp[0].w + mvp[0].y;
      m_planes[BOTTOM].y = mvp[1].w + mvp[1].y;
      m_planes[BOTTOM].z = mvp[2].w + mvp[2].y;
      m_planes[BOTTOM].w = mvp[3].w + mvp[3].y;

      m_planes[NEAR].x = mvp[0].w + mvp[0].z;
      m_planes[NEAR].y = mvp[1].w + mvp[1].z;
      m_planes[NEAR].z = mvp[2].w + mvp[2].z;
      m_planes[NEAR].w = mvp[3].w + mvp[3].z;

      m_planes[FAR].x = mvp[0].w - mvp[0].z;
      m_planes[FAR].y = mvp[1].w - mvp[1].z;
      m_planes[FAR].z = mvp[2].w - mvp[2].z;
      m_planes[FAR].w = mvp[3].w - mvp[3].z;

      for( auto i = 0; i < Plane::COUNT; i++ )
      {
         float length = sqrtf(
             m_planes[i].x * m_planes[i].x + m_planes[i].y * m_planes[i].y +
             m_planes[i].z * m_planes[i].z );
         m_planes[i] /= length;
      }
   }

  private:
   // 0: top plane
   // 1: bottom plane
   // 2: left plane
   // 3: right plane
   // 4: near plane
   // 5: far plane
   glm::vec4 m_planes[Plane::COUNT];
};
}
