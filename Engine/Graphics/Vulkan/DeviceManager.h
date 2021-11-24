#pragma once

#include <Common/Include.h>

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPhysicalDevice );

namespace CYD
{
class Window;
}

namespace vk
{
class Instance;
class Surface;
class Device;
class Swapchain;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class DeviceManager final
{
  public:
   DeviceManager( const CYD::Window& window, const Instance& instance, const Surface& surface );
   ~DeviceManager();

   // Main device is always the first one for now
   Device& getMainDevice() const;
   Swapchain& getMainSwapchain() const;

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   bool _checkDevice( const Surface& surface, const VkPhysicalDevice& physDevice );
   bool _checkDeviceExtensionSupport( const VkPhysicalDevice& physDevice );

   // =============================================================================================
   // Private Variables
   // =============================================================================================
   const CYD::Window& m_window;

   std::vector<const char*> m_extensions;

   std::vector<std::unique_ptr<Device>> m_devices;

   const Instance& m_instance;
   const Surface& m_surface;
};
}
