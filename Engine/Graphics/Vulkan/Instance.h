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
   NON_COPIABLE( Instance )
   explicit Instance( const cyd::Window& window );
   ~Instance();

   const VkInstance& getVKInstance() const { return m_vkInstance; }
   const std::vector<const char*> getLayers() const { return m_layers; }

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _createVKInstance();
   void _createDebugMessenger();

   // =============================================================================================
   // Private Variables
   // =============================================================================================
   std::vector<const char*> m_layers;

   const cyd::Window& m_window;

   VkInstance m_vkInstance                   = nullptr;
   VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
};
}
