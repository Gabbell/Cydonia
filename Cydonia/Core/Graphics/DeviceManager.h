#pragma once

#include <Core/Common/Common.h>

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
class DeviceManager
{
  public:
   explicit DeviceManager( const Instance& instance, const Window& window, const Surface& surface );
   ~DeviceManager();

   // Swapchain created on the main device (first device that supports presentation)
   const Swapchain* getMainSwapchain() const noexcept { return _mainSwapchain; };

   // Main device is always the first one for now
   const Device& getMainDevice() const { return *_devices[0]; };

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

   const Swapchain* _mainSwapchain;

   const Instance& _instance;
   const Window& _window;
   const Surface& _surface;
};
}
