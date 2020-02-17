#include <ECS/Systems/InputSystem.h>

#include <Window/GLFWWindow.h>

#include <ECS/ECS.h>
#include <ECS/SharedComponents/InputComponent.h>

#include <GLFW/glfw3.h>

namespace cyd
{
bool InputSystem::init()
{
   // Settings instance of input interpreter to this window
   // TODO Maybe there's a better way to do this?
   glfwSetWindowUserPointer( m_window.getGLFWwindow(), this );

   // Callback wrappers
   auto mainKeyCallback = []( GLFWwindow* window, int key, int scancode, int action, int mods ) {
      static_cast<InputSystem*>( glfwGetWindowUserPointer( window ) )
          ->_keyCallback( window, key, scancode, action, mods );
   };

   auto mainCursorCallback = []( GLFWwindow* window, double xpos, double ypos ) {
      static_cast<InputSystem*>( glfwGetWindowUserPointer( window ) )
          ->_cursorCallback( window, xpos, ypos );
   };

   auto mainMouseCallback = []( GLFWwindow* window, int button, int action, int mods ) {
      static_cast<InputSystem*>( glfwGetWindowUserPointer( window ) )
          ->_mouseCallback( window, button, action, mods );
   };

   // Registering callbacks
   glfwSetKeyCallback( m_window.getGLFWwindow(), mainKeyCallback );
   glfwSetCursorPosCallback( m_window.getGLFWwindow(), mainCursorCallback );
   glfwSetMouseButtonCallback( m_window.getGLFWwindow(), mainMouseCallback );

   return true;
}

void InputSystem::tick( double /*deltaS*/ )
{
   InputComponent& input = ECS::GetSharedComponent<InputComponent>();
   input.cursorDelta     = glm::vec2( 0.0f );

   // Polling GLFW to trigger the callbacks
   glfwPollEvents();
}

void InputSystem::_keyCallback(
    GLFWwindow* window,
    int key,
    int /*scancode*/,
    int action,
    int /*mods*/ )
{
   if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
   {
      glfwSetWindowShouldClose( window, true );
   }

   InputComponent& input = ECS::GetSharedComponent<InputComponent>();

   if( key == GLFW_KEY_W )
   {
      if( action == GLFW_PRESS )
      {
         input.goingForwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingForwards = false;
      }
   }

   if( key == GLFW_KEY_S )
   {
      if( action == GLFW_PRESS )
      {
         input.goingBackwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingBackwards = false;
      }
   }

   if( key == GLFW_KEY_A )
   {
      if( action == GLFW_PRESS )
      {
         input.goingLeft = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingLeft = false;
      }
   }

   if( key == GLFW_KEY_D )
   {
      if( action == GLFW_PRESS )
      {
         input.goingRight = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingRight = false;
      }
   }
}

void InputSystem::_cursorCallback( GLFWwindow* /*window*/, double xpos, double ypos )
{
   InputComponent& input = ECS::GetSharedComponent<InputComponent>();

   glm::vec2 curPos( xpos, ypos );

   if( input.rotating )
   {
      input.cursorDelta = input.lastCursorPos - curPos;
   }

   input.lastCursorPos = curPos;
}

void InputSystem::_mouseCallback( GLFWwindow* window, int button, int action, int /*mods*/ )
{
   InputComponent& input = ECS::GetSharedComponent<InputComponent>();

   if( button == GLFW_MOUSE_BUTTON_RIGHT )
   {
      if( action == GLFW_PRESS )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
         input.rotating = true;
      }
      else if( action == GLFW_RELEASE )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
         input.rotating = false;
      }
   }
}
}
