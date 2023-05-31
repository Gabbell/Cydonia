#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>

#include <string_view>

namespace CYD
{
class CameraComponent final : public BaseComponent
{
  public:
   CameraComponent() = default;
   CameraComponent( std::string_view name ) : viewName( name ) {}
   COPIABLE( CameraComponent );
   virtual ~CameraComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::CAMERA;

   // Projection mode
   enum class ProjectionMode
   {
      UNKNOWN,
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   std::string_view viewName = "";

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
