#pragma once

#include <Core/Common/Common.h>

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkInstance );
FWDHANDLE( VkDebugUtilsMessengerEXT );
struct VkDebugUtilsMessengerCreateInfoEXT;

namespace cyd
{
class Window;
class DeviceManager;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Instance
{
  public:
   Instance( const Window& window );
   ~Instance();

   const VkInstance& getVKInstance() const { return _vkInstance; }
   const std::vector<const char*> getLayers() const { return _layers; }

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _createVKInstance();
   void _createDebugMessenger();

   // =============================================================================================
   // Private Variables
   // =============================================================================================
   std::vector<const char*> _layers;

   const Window& _window;

   VkInstance _vkInstance                   = nullptr;
   VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
};
}
