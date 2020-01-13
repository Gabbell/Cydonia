#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Common/Include.h>

namespace cyd
{
class LensComponent final : public BaseComponent
{
  public:
   LensComponent() = default;
   COPIABLE( LensComponent );
   virtual ~LensComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::LENS;

   bool init( const void* pDescription ) override;

   enum class ProjectionMode
   {
      UNKNOWN,
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   ProjectionMode projMode = ProjectionMode::UNKNOWN;

   // Projection
   float fov         = 60.0f;
   float aspectRatio = 16.0f / 9.0f;  // 16:9
   float near        = 0.001f;
   float far         = 100.0f;

   // Orthographic
   float left   = 0.0f;
   float right  = 0.0f;
   float bottom = 0.0f;
   float top    = 0.0f;
};
}
