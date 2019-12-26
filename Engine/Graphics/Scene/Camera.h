#pragma once

#include <Graphics/Scene/Transform.h>

#include <glm/glm.hpp>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
struct Transform;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Camera
{
  public:
   // Perspective Constructors
   Camera();
   Camera( float fov, float aspectRatio, float near, float far );
   Camera( const glm::vec3& origin, float fov, float aspectRatio, float near, float far );

   // Orthographic Constructor
   Camera(
       const glm::vec3& origin,
       float left,
       float right,
       float bottom,
       float top,
       float near,
       float far );

   ~Camera();

   void updateVP();

   glm::mat4 getVP() const { return m_proj * m_view; }
   const glm::mat4& getProjectionMatrix() const noexcept { return m_proj; }
   const glm::mat4& getViewMatrix() const noexcept { return m_view; }

   // Projection mode
   void usePerspective();
   void useOrthographic();

   Transform transform;

  private:
   enum class ProjectionMode
   {
      UNKNOWN,
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   ProjectionMode m_projMode = ProjectionMode::UNKNOWN;
   glm::mat4 m_proj          = glm::mat4( 1.0f );
   glm::mat4 m_view          = glm::mat4( 1.0f );

   // Projection
   float m_fov         = 0.0f;
   float m_aspectRatio = 0.0f;
   float m_near        = 0.0f;
   float m_far         = 0.0f;

   // Orthographic
   float m_left   = 0.0f;
   float m_right  = 0.0f;
   float m_bottom = 0.0f;
   float m_top    = 0.0f;
};
}
