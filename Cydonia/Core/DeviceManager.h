#pragma once

#include "Common/Common.h"

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPhysicalDevice );

namespace cyd
{
class Instance;
class Device;
class Window;
}  // namespace cyd

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class DeviceManager
{
  public:
   explicit DeviceManager( const Instance* instance );
   ~DeviceManager();

  private:
   bool _checkDevice( const VkPhysicalDevice& physDevice );

   // Devices used for operations
   std::vector<std::unique_ptr<Device>> _devices;

   // Instance used to create the device manager
   const Instance* _attachedInstance = nullptr;
};
}
