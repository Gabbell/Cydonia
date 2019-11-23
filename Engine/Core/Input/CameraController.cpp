#include <Core/Input/CameraController.h>

#include <Core/Window/GLFWWindow.h>

#include <Core/Graphics/Scene/Camera.h>
#include <Core/Graphics/Scene/Transform.h>

#include <GLFW/glfw3.h>

#include <iostream>

cyd::CameraController::CameraController( Camera& camera ) : _camera( camera ) {}

void cyd::CameraController::interpret()
{
   glm::vec3 displacement( 0.0f );
   if( _goingForwards )
   {
      displacement.z -= MOVE_SPEED;
   }
   if( _goingBackwards )
   {
      displacement.z += MOVE_SPEED;
   }
   if( _goingRight )
   {
      displacement.x += MOVE_SPEED;
   }
   if( _goingLeft )
   {
      displacement.x -= MOVE_SPEED;
   }
   if( displacement != glm::vec3( 0.0f ) )
   {
      _camera.transform->translateLocal( displacement );
   }

   if( _rotating )
   {
      glm::vec2 rotationAngles = _cursorDelta * MOUSE_SENS;

      _camera.transform->rotateLocal( -rotationAngles.y, 0, 0 );
      _camera.transform->rotate( 0, rotationAngles.x, 0 );
      _cursorDelta = glm::vec2( 0.0f );
   }

   _camera.updateVP();
}

void cyd::CameraController::keyCallback(
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
         _goingForwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         _goingForwards = false;
      }
   }

   if( key == GLFW_KEY_S )
   {
      if( action == GLFW_PRESS )
      {
         _goingBackwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         _goingBackwards = false;
      }
   }

   if( key == GLFW_KEY_A )
   {
      if( action == GLFW_PRESS )
      {
         _goingLeft = true;
      }
      else if( action == GLFW_RELEASE )
      {
         _goingLeft = false;
      }
   }

   if( key == GLFW_KEY_D )
   {
      if( action == GLFW_PRESS )
      {
         _goingRight = true;
      }
      else if( action == GLFW_RELEASE )
      {
         _goingRight = false;
      }
   }
}

void cyd::CameraController::cursorCallback( GLFWwindow* /*window*/, double xpos, double ypos )
{
   if( _rotating )
   {
      _cursorDelta = _lastCursorPos - glm::vec2( xpos, ypos );
   }

   _lastCursorPos = glm::vec2( xpos, ypos );
}

void cyd::CameraController::mouseCallback(
    GLFWwindow* window,
    int button,
    int action,
    int /*mods*/ )
{
   if( button == GLFW_MOUSE_BUTTON_RIGHT )
   {
      if( action == GLFW_PRESS )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
         _rotating = true;
      }
      else if( action == GLFW_RELEASE )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
         _rotating = false;
      }
   }
}
