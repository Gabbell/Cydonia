#include <HID/InputInterpreter.h>

#include <Window/GLFWWindow.h>

#include <HID/Controller.h>

#include <GLFW/glfw3.h>

namespace cyd
{
bool InputInterpreter::init( const Window& window )
{
   m_window = &window;

   // Settings instance of input interpreter to this window
   // TODO Maybe there's a better way to do this?
   glfwSetWindowUserPointer( m_window->getGLFWwindow(), this );

   // Callback wrappers
   auto mainKeyCallback = []( GLFWwindow* window, int key, int scancode, int action, int mods ) {
      static_cast<InputInterpreter*>( glfwGetWindowUserPointer( window ) )
          ->mainKeyCallback( window, key, scancode, action, mods );
   };

   auto mainCursorCallback = []( GLFWwindow* window, double xpos, double ypos ) {
      static_cast<InputInterpreter*>( glfwGetWindowUserPointer( window ) )
          ->mainCursorCallback( window, xpos, ypos );
   };

   auto mainMouseCallback = []( GLFWwindow* window, int button, int action, int mods ) {
      static_cast<InputInterpreter*>( glfwGetWindowUserPointer( window ) )
          ->mainMouseCallback( window, button, action, mods );
   };

   // Registering callbacks
   glfwSetKeyCallback( m_window->getGLFWwindow(), mainKeyCallback );
   glfwSetCursorPosCallback( m_window->getGLFWwindow(), mainCursorCallback );
   glfwSetMouseButtonCallback( m_window->getGLFWwindow(), mainMouseCallback );

   return true;
}

void InputInterpreter::tick()
{
   glfwPollEvents();
   for( const auto& controller : m_controllers )
   {
      controller->interpret();
   }
}

void InputInterpreter::mainKeyCallback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods )
{
   if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
   {
      glfwSetWindowShouldClose( window, true );
   }

   for( const auto& controller : m_controllers )
   {
      controller->keyCallback( window, key, scancode, action, mods );
   }
}

void InputInterpreter::mainCursorCallback( GLFWwindow* window, double xpos, double ypos )
{
   for( const auto& controller : m_controllers )
   {
      controller->cursorCallback( window, xpos, ypos );
   }
}

void InputInterpreter::mainMouseCallback( GLFWwindow* window, int button, int action, int mods )
{
   for( const auto& controller : m_controllers )
   {
      controller->mouseCallback( window, button, action, mods );
   }
}
}
