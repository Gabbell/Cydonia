#pragma once

#include <Core/Common/Include.h>

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPhysicalDevice );

namespace cyd
{
class Instance;
class Window;
class Surface;
class Device;
class Swapchain;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class DeviceHerder
{
  public:
   explicit DeviceHerder( const Instance& instance, const Window& window, const Surface& surface );
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
   std::vector<const char*> _extensions;

   std::vector<std::unique_ptr<Device>> _devices;

   const Instance& _instance;
   const Window& _window;
   const Surface& _surface;
};
}
