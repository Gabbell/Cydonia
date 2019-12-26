#pragma once

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
struct GLFWwindow;

namespace cyd
{
class Window;
class Controller;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class InputInterpreter final
{
  public:
   InputInterpreter()  = default;
   ~InputInterpreter() = default;

   bool init( const Window& window );

   void tick();

   void mainKeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );
   void mainCursorCallback( GLFWwindow* window, double xpos, double ypos );
   void mainMouseCallback( GLFWwindow* window, int button, int action, int mods );

   // Call controller callbacks based on polymorphism. Consider the vtable cost.
   // Alternatives: Delegates/Function pointers, need to compare performance
   void addController( Controller& controller ) { m_controllers.push_back( &controller ); }

  private:
   std::vector<Controller*> m_controllers;

   const Window* m_window;
};
}
