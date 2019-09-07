#pragma once

// Enable the WSI extensions
#if defined( __linux__ )
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vector>
#include <memory>
#include <string>
#include <cstdint>

// ================================================================================================
// Forwards
// ================================================================================================
struct SDL_Window;
namespace vk
{
class SurfaceKHR;
}
namespace cyd
{
class Instance;
}

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

   void createSurface( const Instance* instance );

   std::vector<const char*> getExtensions() const noexcept { return _extensions; };

  private:
   const std::string _title = "Default Title";

   std::vector<const char*> _extensions;

   // Window is the owner of this SDL_Window
   SDL_Window* _sdlWindow = nullptr;

   std::unique_ptr<vk::SurfaceKHR> _vkSurface;

   uint32_t _width  = 0;
   uint32_t _height = 0;
};
}
