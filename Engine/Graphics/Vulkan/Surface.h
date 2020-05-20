#pragma once

#include <Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSurfaceKHR );

namespace CYD
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
   Surface( const CYD::Window& window, const Instance& instance );
   ~Surface();

   const VkSurfaceKHR& getVKSurface() const { return m_vkSurface; }

  private:
   const CYD::Window& m_window;

   const Instance& m_instance;

   VkSurfaceKHR m_vkSurface = nullptr;
};
}
