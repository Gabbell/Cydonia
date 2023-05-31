#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/SharedComponents/InputComponent.h>

// ================================================================================================
// Forwards
// ================================================================================================
struct GLFWwindow;
namespace CYD
{
class Window;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class InputSystem final : public CommonSystem<InputComponent>
{
  public:
   explicit InputSystem( const Window& window );
   NON_COPIABLE( InputSystem );
   virtual ~InputSystem() = default;

   // The input system always has to tick, regardless of how many entities have an input component
   bool hasToTick() const noexcept override { return true; }
   void tick( double deltaS ) override;

  private:
   void _keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );
   void _cursorCallback( GLFWwindow* window, double xpos, double ypos );
   void _mouseCallback( GLFWwindow* window, int button, int action, int mods );
   void _framebufferSizeCallback( GLFWwindow* window, int width, int height );

   const Window& m_window;
};
}
