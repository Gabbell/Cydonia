#pragma once

#include <ECS/Components/BaseComponent.h>

#include <glm/glm.hpp>

namespace cyd
{
class FreeControllerComponent final : public BaseComponent
{
  public:
   FreeControllerComponent() = default;
   COPIABLE( FreeControllerComponent );
   virtual ~FreeControllerComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::FREECONTROLLER;

   bool init( const void* pDescription ) override;

   // Last cursor pos registered by cursor callback
   glm::vec2 lastCursorPos = glm::vec2( 0.0f );

   // Keeps track of mouse displacement since last update
   glm::vec2 cursorDelta = glm::vec2( 0.0f );

   static constexpr float MOVE_SPEED = 0.01f;
   static constexpr float MOUSE_SENS = 0.001f;

   bool goingForwards  = false;
   bool goingBackwards = false;
   bool goingRight     = false;
   bool goingLeft      = false;
   bool rotating       = false;
};
}
