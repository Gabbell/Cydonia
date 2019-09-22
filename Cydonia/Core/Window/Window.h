#pragma once

// Enable the WSI extensions
#if defined( __linux__ )
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <Core/Graphics/CommonTypes.h>

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

   const Extent& getExtent() const noexcept { return _extent; }
   SDL_Window* getSDLWindow() const noexcept { return _sdlWindow; }
   std::vector<const char*> getExtensionsFromSDL() const noexcept { return _extensions; };

  private:
   const std::string _title = "Default Title";

   std::vector<const char*> _extensions;

   // Window is the owner of this SDL_Window
   SDL_Window* _sdlWindow = nullptr;

   // Dimensions
   Extent _extent = {0, 0};
};
}
