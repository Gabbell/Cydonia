#pragma once

#include <Core/Input/Controller.h>

#include <glm/glm.hpp>

// ================================================================================================
// Forwards
// ================================================================================================
struct GLFWwindow;

namespace cyd
{
class Camera;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class CameraController final : public Controller
{
  public:
   CameraController( Camera& camera );
   ~CameraController() = default;

   void interpret() override;
   void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods ) override;
   void cursorCallback( GLFWwindow* window, double xpos, double ypos ) override;
   void mouseCallback( GLFWwindow* window, int button, int action, int mods ) override;

  private:
   Camera& _camera;

   // Last cursor pos registered by cursor callback
   glm::vec2 _lastCursorPos = glm::vec2( 0.0f );

   // Keeps track of mouse displacement since last update
   glm::vec2 _cursorDelta = glm::vec2( 0.0f );

   static constexpr float MOVE_SPEED = 0.01f;
   static constexpr float MOUSE_SENS = 0.001f;

   // State
   bool _goingForwards  = false;
   bool _goingBackwards = false;
   bool _goingRight     = false;
   bool _goingLeft      = false;
   bool _rotating       = false;
};
}
