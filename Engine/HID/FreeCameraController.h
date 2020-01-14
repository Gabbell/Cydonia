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
};
}
