#pragma once

#include <vector>
#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
namespace vk
{
class Instance;
class SurfaceKHR;
class DispatchLoaderDynamic;
class DebugUtilsMessengerEXT;
struct DebugUtilsMessengerCreateInfoEXT;
}

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

   const vk::Instance& getVKInstance() const { return *_vkInstance; }

   const std::vector<const char*>& getLayers() const { return _layers; }

  private:
   void _createVKInstance( const Window* window );
   void _createDebugMessenger();

   void _populateDebugInfo( vk::DebugUtilsMessengerCreateInfoEXT& debugInfo );

   bool _checkValidationLayerSupport( const std::vector<const char*>& desiredLayers );
   bool _checkExtensionSupport( const std::vector<const char*>& desiredExtensions );

   std::vector<const char*> _layers;

   std::unique_ptr<vk::Instance> _vkInstance;
   std::unique_ptr<vk::DispatchLoaderDynamic> _dld;
   std::unique_ptr<vk::DebugUtilsMessengerEXT> _debugMessenger;
};
}
