#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/SharedComponents/CameraComponent.h>

namespace cyd
{
class CameraSystem : public CommonSystem<TransformComponent, CameraComponent>
{
  public:
   CameraSystem() = default;
   NON_COPIABLE( CameraSystem );
   virtual ~CameraSystem() = default;

   bool init() override { return true; }
   void uninit() override{};
   void tick( double deltaS ) override;

  private:
   enum class ProjectionMode
   {
      UNKNOWN,
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   ProjectionMode _projMode = ProjectionMode::PERSPECTIVE;

   // Planes
   float _near = 0.001f;
   float _far  = 100.0f;

   // Projection
   float _fov         = 60.0f;         // in degrees
   float _aspectRatio = 16.0f / 9.0f;  // 16:9

   // Orthographic
   float _left   = -1.0f;
   float _right  = 1.0f;
   float _bottom = -1.0f;
   float _top    = 1.0f;
};
}
