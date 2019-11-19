#pragma once

// ================================================================================================
// Forwards
// ================================================================================================
struct GLFWwindow;

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Controller
{
  public:
   Controller()          = default;
   virtual ~Controller() = default;

   // Interpret current state of the controller
   virtual void interpret() = 0;

   // Callbacks
   virtual void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods ) = 0;
   virtual void cursorCallback( GLFWwindow* window, double xpos, double ypos )                 = 0;
   virtual void mouseCallback( GLFWwindow* window, int button, int action, int mods )          = 0;
};
}
