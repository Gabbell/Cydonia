#pragma once

#include <Common/Include.h>

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkInstance );

#if CYD_DEBUG
FWDHANDLE( VkDebugUtilsMessengerEXT );
#endif

namespace CYD
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
   explicit Instance( const CYD::Window& window );
   NON_COPIABLE( Instance );
   ~Instance();

   const VkInstance& getVKInstance() const { return m_vkInstance; }
   const std::vector<const char*> getLayers() const { return m_layers; }

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _createVKInstance();

#if CYD_DEBUG
   void _createDebugMessenger();
#endif

   // =============================================================================================
   // Private Variables
   // =============================================================================================
   std::vector<const char*> m_layers;

   const CYD::Window& m_window;

   VkInstance m_vkInstance = nullptr;

#if CYD_DEBUG
   VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
#endif
};
}
