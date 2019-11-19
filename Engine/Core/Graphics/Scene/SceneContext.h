#pragma once

#include <Core/Graphics/Vulkan/Types.h>

#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Camera;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class SceneContext
{
  public:
   SceneContext( const Rectangle& viewport );
   ~SceneContext();

   Camera& getCamera() { return *_camera; }

  private:
   std::unique_ptr<Camera> _camera;
};
}
