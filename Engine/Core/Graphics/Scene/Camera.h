#pragma once

#include <Core/Graphics/Vulkan/Types.h>

#include <glm/glm.hpp>

#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
struct Rectangle;
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
   Camera( const Rectangle& viewport );
   ~Camera();

   void updateVP();

   glm::mat4 getVP() const { return _proj * _view; }
   const glm::mat4& getProjectionMatrix() const noexcept { return _proj; }
   const glm::mat4& getViewMatrix() const noexcept { return _view; }
   const Rectangle& getViewport() const noexcept { return _viewport; }

   void usePerspective();
   void useOrthographic();

   std::unique_ptr<Transform> transform;

  private:
   enum class ProjectionMode
   {
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   Rectangle _viewport = {};

   ProjectionMode _projMode;
   glm::mat4 _proj;
   glm::mat4 _view;

   float _fov;
   float _aspectRatio;
   float _near;
   float _far;
};
}
