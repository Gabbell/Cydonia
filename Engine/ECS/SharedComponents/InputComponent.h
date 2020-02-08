#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <glm/glm.hpp>

namespace cyd
{
class InputComponent final : public BaseSharedComponent
{
public:
   InputComponent() = default;
   NON_COPIABLE( InputComponent )
   virtual ~InputComponent() = default;

   static constexpr SharedComponentType TYPE = SharedComponentType::INPUT;

   static constexpr float MOVE_SPEED = 0.01f;
   static constexpr float MOUSE_SENS = 0.001f;

   // Last cursor pos registered by cursor callback
   glm::vec2 lastCursorPos = glm::vec2( 0.0f );

   // Keeps track of mouse displacement since last update
   glm::vec2 cursorDelta = glm::vec2( 0.0f );

   bool goingForwards  = false;
   bool goingBackwards = false;
   bool goingRight     = false;
   bool goingLeft      = false;
   bool rotating       = false;
};
}
