#pragma once

#include <glm/glm.hpp>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Camera
{
  public:
   Camera();
   Camera( const glm::vec4& origin, float fov, float aspectRatio, float nearPlane, float farPlane );
   ~Camera() = default;

  private:
   glm::vec4 _position;

   float _fov;
   float _aspectRatio;
   float _near;
   float _far;
};
}
