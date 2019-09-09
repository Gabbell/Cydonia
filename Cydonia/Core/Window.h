#pragma once

// Enable the WSI extensions
#if defined( __linux__ )
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
struct SDL_Window;

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

   SDL_Window* getWindow() const noexcept { return _sdlWindow; }
   std::vector<const char*> getExtensions() const noexcept { return _extensions; };

  private:
   const std::string _title = "Default Title";

   std::vector<const char*> _extensions;

   // Window is the owner of this SDL_Window
   SDL_Window* _sdlWindow = nullptr;

   uint32_t _width  = 0;
   uint32_t _height = 0;
};
}
