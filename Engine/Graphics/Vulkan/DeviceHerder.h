#pragma once

#include <Common/Include.h>

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPhysicalDevice );

namespace cyd
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
class DeviceHerder final
{
  public:
   DeviceHerder( const cyd::Window& window, const Instance& instance, const Surface& surface );
   ~DeviceHerder();

   // Main device is always the first one for now
   Device* getMainDevice() const { return _devices[0].get(); }
   const Swapchain* getMainSwapchain();

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   bool _checkDevice( const Surface& surface, const VkPhysicalDevice& physDevice );
   bool _checkDeviceExtensionSupport( const VkPhysicalDevice& physDevice );

   // =============================================================================================
   // Private Variables
   // =============================================================================================
   const cyd::Window& _window;

   std::vector<const char*> _extensions;

   std::vector<std::unique_ptr<Device>> _devices;

   const Instance& _instance;
   const Surface& _surface;
};
}
