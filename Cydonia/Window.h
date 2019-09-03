#pragma once

#include <vulkan/vulkan.hpp>

// Forwards
struct SDL_Window;

namespace cyd
{
class Instance;

// Definition
class Window
{
  public:
   Window( uint32_t width, uint32_t height, const std::string& title, const Instance* instance );
   ~Window();

  private:
   const std::string _title = "Default Title";

   // One surface per window
   VkSurfaceKHR _surface = VK_NULL_HANDLE;

   // Window is the owner of this SDL_Window
   SDL_Window* _sdlWindow = nullptr;

   uint32_t _width  = 0;
   uint32_t _height = 0;
};
}  // namespace cyd
