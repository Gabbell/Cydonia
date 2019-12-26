#include <HID/FreeCameraController.h>

#include <Window/GLFWWindow.h>

#include <Graphics/Scene/Camera.h>
#include <Graphics/Scene/Transform.h>

#include <GLFW/glfw3.h>

namespace cyd
{
FreeCameraController::FreeCameraController( Camera& camera ) : _camera( camera ) {}

void FreeCameraController::interpret()
{
   glm::vec3 displacement( 0.0f );
   if( m_goingForwards )
   {
      displacement.z -= MOVE_SPEED;
   }
   if( m_goingBackwards )
   {
      displacement.z += MOVE_SPEED;
   }
   if( m_goingRight )
   {
      displacement.x += MOVE_SPEED;
   }
   if( m_goingLeft )
   {
      displacement.x -= MOVE_SPEED;
   }
   if( displacement != glm::vec3( 0.0f ) )
   {
      _camera.transform.translateLocal( displacement );
   }

   if( m_rotating )
   {
      glm::vec2 rotationAngles = m_cursorDelta * MOUSE_SENS;

      _camera.transform.rotateLocal( -rotationAngles.y, 0, 0 );
      _camera.transform.rotate( 0, rotationAngles.x, 0 );
      m_cursorDelta = glm::vec2( 0.0f );
   }

   _camera.updateVP();
}

void FreeCameraController::keyCallback(
    GLFWwindow* /*window*/,
    int key,
    int /*scancode*/,
    int action,
    int /*mods*/ )
{
   if( key == GLFW_KEY_W )
   {
      if( action == GLFW_PRESS )
      {
         m_goingForwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         m_goingForwards = false;
      }
   }

   if( key == GLFW_KEY_S )
   {
      if( action == GLFW_PRESS )
      {
         m_goingBackwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         m_goingBackwards = false;
      }
   }

   if( key == GLFW_KEY_A )
   {
      if( action == GLFW_PRESS )
      {
         m_goingLeft = true;
      }
      else if( action == GLFW_RELEASE )
      {
         m_goingLeft = false;
      }
   }

   if( key == GLFW_KEY_D )
   {
      if( action == GLFW_PRESS )
      {
         m_goingRight = true;
      }
      else if( action == GLFW_RELEASE )
      {
         m_goingRight = false;
      }
   }
}

void FreeCameraController::cursorCallback( GLFWwindow* /*window*/, double xpos, double ypos )
{
   if( m_rotating )
   {
      m_cursorDelta = m_lastCursorPos - glm::vec2( xpos, ypos );
   }

   m_lastCursorPos = glm::vec2( xpos, ypos );
}

void FreeCameraController::mouseCallback( GLFWwindow* window, int button, int action, int /*mods*/ )
{
   if( button == GLFW_MOUSE_BUTTON_RIGHT )
   {
      if( action == GLFW_PRESS )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
         m_rotating = true;
      }
      else if( action == GLFW_RELEASE )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
         m_rotating = false;
      }
   }
}
}