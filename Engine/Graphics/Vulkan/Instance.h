#pragma once

#include <Common/Include.h>

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkInstance );
FWDHANDLE( VkDebugUtilsMessengerEXT );

namespace cyd
{
class Window;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Instance final
{
  public:
   NON_COPIABLE( Instance );
   explicit Instance( const cyd::Window& window );
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

   const cyd::Window& _window;

   VkInstance _vkInstance                   = nullptr;
   VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
};
}
