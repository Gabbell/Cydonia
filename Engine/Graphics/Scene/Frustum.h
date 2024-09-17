#pragma once

#include <Graphics/Scene/BoudingBox.h>

#include <Graphics/Handles/ResourceHandle.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class Frustum
{
  public:
   Frustum() = default;
   Frustum( const glm::mat4& projMat, const glm::mat4& viewMat ) { update( projMat, viewMat ); }
   ~Frustum() = default;

   enum Plane
   {
      LEFT,
      RIGHT,
      BOTTOM,
      TOP,
      NEAR,
      FAR,
      PLANE_COUNT
   };

   enum Corner
   {
      NEAR_BOTTOM_LEFT,
      NEAR_TOP_LEFT,
      NEAR_TOP_RIGHT,
      NEAR_BOTTOM_RIGHT,
      FAR_BOTTOM_LEFT,
      FAR_TOP_LEFT,
      FAR_TOP_RIGHT,
      FAR_BOTTOM_RIGHT,
      CORNER_COUNT
   };

   const glm::vec4& getPlane( Plane planeIndex ) const { return m_planes[planeIndex]; }

   // In world space
   const glm::vec3& getCorner( Corner cornerIndex ) const { return m_corners[cornerIndex]; }

   BoundingBox getWorldAABB() const;

   void update( const glm::mat4& projMat, const glm::mat4& viewMat );
   void draw( CmdListHandle cmdList ) const;

  private:
   glm::vec4 m_planes[PLANE_COUNT];
   glm::vec3 m_corners[CORNER_COUNT];
};
}
