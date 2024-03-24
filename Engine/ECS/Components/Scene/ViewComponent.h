#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/Components/ComponentTypes.h>

#include <ECS/Entity.h>

#include <glm/glm.hpp>

#include <string_view>

namespace CYD
{
class ViewComponent final : public BaseComponent
{
  public:
   // Projection mode
   enum class ProjectionMode
   {
      UNKNOWN,
      PERSPECTIVE,
      ORTHOGRAPHIC
   };

   ViewComponent() = default;
   ViewComponent( EntityHandle fitToEntityView, bool reverseZ = false );
   ViewComponent( float fov, float near, float far, bool reverseZ = false );
   ViewComponent(
       float left,
       float right,
       float bottom,
       float top,
       float near,
       float far,
       bool reverseZ = false );
   COPIABLE( ViewComponent );
   virtual ~ViewComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::VIEW;

   // =============================================================================================
   using ViewIndex                               = uint32_t;
   static constexpr ViewIndex INVALID_VIEW_INDEX = 0xFFFFFFFF;
   uint32_t index                                = INVALID_VIEW_INDEX;

   // =============================================================================================
   // Common
   float near = 0.1f;
   float far  = 32000.0f;  // 32KM

   glm::mat4 viewMat = glm::mat4( 1.0f );
   glm::mat4 projMat = glm::mat4( 1.0f );

   // =============================================================================================
   struct OrthographicParams
   {
      float left;
      float right;
      float bottom;
      float top;
   };

   struct PerspectiveParams
   {
      float fov;  // In degrees
   };

   union ProjectionParams
   {
      ProjectionParams() = default;

      OrthographicParams ortho;
      PerspectiveParams perspective;
   } params;

   ProjectionMode projMode = ProjectionMode::UNKNOWN;

   // =============================================================================================
   EntityHandle fitToEntity = Entity::INVALID_ENTITY;

   bool reverseZ = false;

#if CYD_DEBUG
   bool updateDebugFrustum       = true;
   glm::mat4 debugInverseViewMat = glm::mat4( 1.0f );
   glm::mat4 debugInverseProjMat = glm::mat4( 1.0f );
#endif
};
}
