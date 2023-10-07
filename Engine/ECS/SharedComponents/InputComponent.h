#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <glm/glm.hpp>

namespace CYD
{
class InputComponent final : public BaseSharedComponent
{
  public:
   InputComponent() = default;
   NON_COPIABLE( InputComponent );
   virtual ~InputComponent() = default;

   static constexpr SharedComponentType TYPE = SharedComponentType::INPUT;

   glm::vec2 curCursorPos = glm::vec2( 0.0f );
   glm::vec2 cursorDelta  = glm::vec2( 0.0f );

   bool goingForwards  = false;
   bool goingBackwards = false;
   bool goingRight     = false;
   bool goingLeft      = false;
   bool goingUp        = false;
   bool goingDown      = false;
   bool rightClick     = false;
   bool sprinting      = false;
};
}
