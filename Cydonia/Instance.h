#pragma once

#include <vulkan/vulkan.hpp>

namespace cyd
{
// Definition
class Instance
{
  public:
   Instance();
   ~Instance();

   const vk::Instance& getVKInstance() const noexcept { return _vkInstance; }
   const vk::DispatchLoaderDynamic& getDLD() const noexcept { return _dld; }

   const std::vector<const char*>& getLayers() const { return _layers; }

  private:
   // Functions
   void _createVKInstance();
   void _createDebugMessenger();

   void _populateExtensions();
   void _populateDebugInfo( vk::DebugUtilsMessengerCreateInfoEXT& debugInfo );

   bool _checkValidationLayerSupport( const std::vector<const char*>& desiredLayers );
   bool _checkExtensionSupport( const std::vector<const char*>& desiredExtensions );

   // Members
   std::vector<const char*> _layers;
   std::vector<const char*> _extensions;

   // TODO Figure out what exactly this does
   vk::DispatchLoaderDynamic _dld;

   vk::Instance _vkInstance;
   vk::DebugUtilsMessengerEXT _debugMessenger;
};
}  // namespace cyd
