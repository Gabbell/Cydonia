#pragma once

#include "Device.h"

#include <vector>

namespace cyd
{
// Forwards
class Instance;

// Definition
class DeviceManager
{
  public:
   DeviceManager( const Instance* instance );
   ~DeviceManager();

  private:
   bool _checkDevice( const vk::PhysicalDevice& physDevice );

   std::vector<Device> _devices;

   // Instance used to create the device manager
   const Instance* _attachedInstance = nullptr;
};
}  // namespace cyd
