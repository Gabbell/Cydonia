#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <glm/glm.hpp>

namespace CYD
{
class CameraComponent final : public BaseSharedComponent
{
  public:
   CameraComponent() = default;
   NON_COPIABLE( CameraComponent );
   virtual ~CameraComponent() = default;

   static constexpr SharedComponentType TYPE = SharedComponentType::CAMERA;

   struct ViewProjection
   {
      glm::mat4 view = glm::mat4( 1.0f );
      glm::mat4 proj = glm::mat4( 1.0f );
   } vp;

   glm::vec4 pos;

   enum class ProjectionMode
   {
      UNKNOWN,
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   ProjectionMode projMode = ProjectionMode::PERSPECTIVE;

   // Planes
   float near = 0.1f;
   float far  = 1000.0f;

   // Projection
   float fov         = 60.0f;         // in degrees
   float aspectRatio = 16.0f / 9.0f;  // 16:9

   // Orthographic
   float left   = -1.0f;
   float right  = 1.0f;
   float bottom = -1.0f;
   float top    = 1.0f;
};
}
