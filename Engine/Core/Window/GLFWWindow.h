#pragma once

// Enable the WSI extensions
#if defined( __linux__ )
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <Core/Graphics/Vulkan/Types.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
struct GLFWwindow;

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Window
{
  public:
   Window( uint32_t width, uint32_t height, const std::string& title );
   ~Window();

   bool isRunning() const;
   const Extent& getExtent() const noexcept { return _extent; }
   GLFWwindow* getGLFWwindow() const noexcept { return _glfwWindow; }
   std::vector<const char*> getExtensionsFromGLFW() const noexcept { return _extensions; };

  private:
   const std::string _title = "Default Title";

   std::vector<const char*> _extensions;

   // Window is the owner of this GLFWwindow
   GLFWwindow* _glfwWindow = nullptr;

   // Dimensions
   Extent _extent = { 0, 0 };
};
}
