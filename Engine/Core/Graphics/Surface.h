#pragma once

#include <Core/Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSurfaceKHR );

namespace cyd
{
class Window;
class Instance;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Surface
{
  public:
   Surface( const Instance& instance, const Window& window );
   ~Surface();

   const VkSurfaceKHR& getVKSurface() const { return _vkSurface; }

  private:
   const Window& _window;
   const Instance& _instance;

   VkSurfaceKHR _vkSurface = nullptr;
};
}
