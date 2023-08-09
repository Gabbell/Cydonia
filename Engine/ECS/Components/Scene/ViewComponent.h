#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/Components/ComponentTypes.h>

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
   ViewComponent( std::string_view name ) : name( name ) {}
   ViewComponent( std::string_view name, float fov, float aspectRatio, float near, float far )
       : name( name ),
         projMode( ProjectionMode::PERSPECTIVE ),
         near( near ),
         far( far ),
         fov( fov ),
         aspectRatio( aspectRatio )
   {
   }
   ViewComponent(
       std::string_view name,
       float left,
       float right,
       float bottom,
       float top,
       float near,
       float far )
       : name( name ),
         projMode( ProjectionMode ::ORTHOGRAPHIC ),
         near( near ),
         far( far ),
         left( left ),
         right( right ),
         bottom( bottom ),
         top( top )
   {
   }
   COPIABLE( ViewComponent );
   virtual ~ViewComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::VIEW;

   std::string_view name = "";

   ProjectionMode projMode = ProjectionMode::PERSPECTIVE;

   // Planes
   float near = 0.1f;
   float far  = 10000.0f;

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
