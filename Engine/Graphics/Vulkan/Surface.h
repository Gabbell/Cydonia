#pragma once

#include <Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSurfaceKHR );

namespace cyd
{
class Window;
}

namespace vk
{
class Instance;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Surface final
{
  public:
   Surface( const cyd::Window& window, const Instance& instance );
   ~Surface();

   const VkSurfaceKHR& getVKSurface() const { return _vkSurface; }

  private:
   const cyd::Window& _window;

   const Instance& _instance;

   VkSurfaceKHR _vkSurface = nullptr;
};
}
