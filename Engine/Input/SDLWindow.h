#pragma once

// Enable the WSI extensions
#if defined( __linux__ )
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <Graphics/GraphicsTypes.h>

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
namespace CYD
{
class Window
{
  public:
   Window() = default;
   ~Window();

   bool init( uint32_t width, uint32_t height, const std::string& title );
   bool uninit();

   const Extent2D& getExtent2D() const noexcept { return m_extent; }
   SDL_Window* getSDLWindow() const noexcept { return m_sdlWindow; }
   std::vector<const char*> getExtensionsFromSDL() const noexcept { return _extensions; };

  private:
   const std::string m_title = "Default Title";

   std::vector<const char*> _extensions;

   // Window is the owner of this SDLm_Window
   SDL_Window* m_sdlWindow = nullptr;

   // Dimensions
   Extent2D m_extent = { 0, 0 };
};
}
