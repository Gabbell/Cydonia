#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/SharedComponents/SharedComponentType.h>

#include <glm/glm.hpp>

namespace CYD
{
class CameraComponent final : public BaseSharedComponent
{
  public:
   CameraComponent();
   NON_COPIABLE( CameraComponent );
   virtual ~CameraComponent();

   static constexpr SharedComponentType TYPE = SharedComponentType::CAMERA;

   // GPU Buffers
   BufferHandle viewBuffer;

   struct EnvironmentView
   {
      glm::vec4 position = glm::vec4( 0.0f );
      glm::mat4 viewMat  = glm::mat4( 1.0f );
      glm::mat4 projMat  = glm::mat4( 1.0f );
   } view;

   // Projection mode
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
