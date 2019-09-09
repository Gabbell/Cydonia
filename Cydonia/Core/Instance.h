#pragma once

#include "Common/Common.h"

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkInstance );
FWDHANDLE( VkSurfaceKHR );
FWDHANDLE( VkDebugUtilsMessengerEXT );
struct VkDebugUtilsMessengerCreateInfoEXT;

namespace cyd
{
class Window;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Instance
{
  public:
   Instance( const Window* window );
   ~Instance();

   const std::vector<const char*>& getLayers() const { return _layers; }
   const VkInstance getVKInstance() const { return _vkInstance; }
   const VkSurfaceKHR getSurface() const noexcept { return _vkSurface; }

  private:
   void _createVKInstance( const Window* window );
   void _createVKSurface( const Window* window );
   void _createDebugMessenger();

   void _populateDebugInfo( VkDebugUtilsMessengerCreateInfoEXT& debugInfo );

   bool _checkValidationLayerSupport( const std::vector<const char*>& desiredLayers );

   std::vector<const char*> _layers;

   VkInstance _vkInstance                   = nullptr;
   VkSurfaceKHR _vkSurface                  = nullptr;
   VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
};
}
