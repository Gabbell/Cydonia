#pragma once

#include <Graphics/Scene/Camera.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class SceneContext final
{
  public:
   SceneContext() = default;
   ~SceneContext()       = default;

   Camera& getCamera() { return _camera; }

  private:
   Camera _camera;
};
}
