// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include "Window.h"

#include "Assert.h"
#include "Instance.h"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

cyd::Window::Window( uint32_t width, uint32_t height, const std::string& title )
    : _width( width ), _height( height )
{
   if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
   {
      CYDASSERT( !SDL_GetError() );
   }

   // Creating SDL_Window
   _sdlWindow = SDL_CreateWindow(
       title.c_str(),
       SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED,
       _width,
       _height,
       SDL_WINDOW_VULKAN );
   CYDASSERT( _sdlWindow && "Could not create SDL window" );

   // Populating extensions
   uint32_t extensionsCount = 0;
   CYDASSERT( SDL_Vulkan_GetInstanceExtensions( _sdlWindow, &extensionsCount, nullptr ) );

   _extensions.reserve( extensionsCount );
   CYDASSERT(
       SDL_Vulkan_GetInstanceExtensions( _sdlWindow, &extensionsCount, _extensions.data() ) );

   _extensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
   _extensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );

#if _DEBUG
   _extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif
}

void cyd::Window::createSurface( const Instance* instance )
{
   // Creating surface
   SDL_SysWMinfo wmInfo;
   SDL_GetWindowWMInfo( _sdlWindow, &wmInfo );

   if( instance )
   {
      const vk::Instance& vkInstance = instance->getVKInstance();

#ifdef _WIN32
      auto result = vkInstance.createWin32SurfaceKHR( vk::Win32SurfaceCreateInfoKHR(
          vk::Win32SurfaceCreateFlagsKHR(), wmInfo.info.win.hinstance, wmInfo.info.win.window ) );

      CYDASSERT(
          result.result == vk::Result::eSuccess && "Window:: Could not create WIN32 surface" );
#endif

      _vkSurface = std::make_unique<vk::SurfaceKHR>( std::move( result.value ) );
   }
}

cyd::Window::~Window()
{
   SDL_DestroyWindow( _sdlWindow );
   SDL_Quit();
}
