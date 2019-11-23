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
   InputInterpreter( const Window& window );
   ~InputInterpreter() = default;

   void pollAndInterpret();

   void mainKeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );
   void mainCursorCallback( GLFWwindow* window, double xpos, double ypos );
   void mainMouseCallback( GLFWwindow* window, int button, int action, int mods );

   // Call controller callbacks based on polymorphism. Consider the vtable cost.
   // Alternatives: Delegates/Function pointers, need to compare performance
   void addController( Controller& controller ) { _controllers.push_back( &controller ); }

  private:
   std::vector<Controller*> _controllers;

   const Window& _window;
};
}
