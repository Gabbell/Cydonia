#pragma once

#include <Common/Include.h>

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
namespace d3d12
{
class Factory;
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace d3d12
{
class DeviceManager final
{
  public:
   DeviceManager( const Factory& factory );
   ~DeviceManager();

   // Main device is always the first one for now
   Device& getMainDevice() const;

  private:
   // =============================================================================================
   // Private Variables
   // =============================================================================================
   const Factory& m_factory;

   std::vector<std::unique_ptr<Device>> m_devices;
};
}
