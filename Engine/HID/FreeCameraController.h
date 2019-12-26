#pragma once

#include <HID/Controller.h>

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
class FreeCameraController final : public Controller
{
  public:
   FreeCameraController( Camera& camera );
   ~FreeCameraController() = default;

   void interpret() override;
   void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods ) override;
   void cursorCallback( GLFWwindow* window, double xpos, double ypos ) override;
   void mouseCallback( GLFWwindow* window, int button, int action, int mods ) override;

  private:
   Camera& _camera;

   // Last cursor pos registered by cursor callback
   glm::vec2 m_lastCursorPos = glm::vec2( 0.0f );

   // Keeps track of mouse displacement since last update
   glm::vec2 m_cursorDelta = glm::vec2( 0.0f );

   static constexpr float MOVE_SPEED = 0.01f;
   static constexpr float MOUSE_SENS = 0.001f;

   // State
   bool m_goingForwards  = false;
   bool m_goingBackwards = false;
   bool m_goingRight     = false;
   bool m_goingLeft      = false;
   bool m_rotating       = false;
};
}
