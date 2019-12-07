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

   glm::mat4 getVP() const { return _proj * _view; }
   const glm::mat4& getProjectionMatrix() const noexcept { return _proj; }
   const glm::mat4& getViewMatrix() const noexcept { return _view; }

   // Projection mode
   void usePerspective();
   void useOrthographic();

   Transform transform;

  private:
   enum class ProjectionMode
   {
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   ProjectionMode _projMode;
   glm::mat4 _proj;
   glm::mat4 _view;

   // Projection
   float _fov         = 0.0f;
   float _aspectRatio = 0.0f;
   float _near        = 0.0f;
   float _far         = 0.0f;

   // Orthographic
   float _left   = 0.0f;
   float _right  = 0.0f;
   float _bottom = 0.0f;
   float _top    = 0.0f;
};
}
