#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/SharedComponents/InputComponent.h>

// ================================================================================================
// Forwards
// ================================================================================================
struct GLFWwindow;
namespace cyd
{
class Window;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class InputSystem final : public CommonSystem<InputComponent>
{
  public:
   InputSystem() = delete;
   InputSystem( const Window& window ) : m_window( window ) {}
   NON_COPIABLE( InputSystem )
   virtual ~InputSystem() = default;

   bool init() override;
   void uninit() override{};
   void tick( double deltaS ) override;

  private:
   void _keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );
   void _cursorCallback( GLFWwindow* window, double xpos, double ypos );
   void _mouseCallback( GLFWwindow* window, int button, int action, int mods );

   const Window& m_window;
};
}
