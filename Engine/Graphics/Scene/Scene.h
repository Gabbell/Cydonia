#pragma once

#include <Graphics/Scene/Camera.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Scene
{
  public:
   Scene();
   ~Scene();

   Camera& getCamera() { return _camera; }

  private:
   Camera _camera;
};
}
