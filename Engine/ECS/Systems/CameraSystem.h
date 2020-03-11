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

   ProjectionMode m_projMode = ProjectionMode::PERSPECTIVE;

   // Planes
   float m_near = 0.001f;
   float m_far  = 1000.0f;

   // Projection
   float m_fov         = 60.0f;         // in degrees
   float m_aspectRatio = 16.0f / 9.0f;  // 16:9

   // Orthographic
   float m_left   = -1.0f;
   float m_right  = 1.0f;
   float m_bottom = -1.0f;
   float m_top    = 1.0f;
};
}
