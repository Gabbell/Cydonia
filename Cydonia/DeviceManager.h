#pragma once

#include "Device.h"

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Instance;
class Device;
}

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
   bool _checkDevice( const vk::PhysicalDevice& physDevice );

   // Devices used for operations
   std::vector<std::unique_ptr<Device>> _devices;

   // Instance used to create the device manager
   const Instance* _attachedInstance = nullptr;
};
}
