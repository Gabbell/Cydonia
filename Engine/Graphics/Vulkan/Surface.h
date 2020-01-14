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

   const VkSurfaceKHR& getVKSurface() const { return m_vkSurface; }

  private:
   const cyd::Window& m_window;

   const Instance& m_instance;

   VkSurfaceKHR m_vkSurface = nullptr;
};
}
