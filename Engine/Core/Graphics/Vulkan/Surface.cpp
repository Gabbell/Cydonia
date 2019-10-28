#include <Core/Graphics/Vulkan/Surface.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Vulkan/Instance.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

cyd::Surface::Surface( const Instance& instance, const Window& window )
    : _window( window ), _instance( instance )
{
   // Creating surface
   if( !SDL_Vulkan_CreateSurface( window.getSDLWindow(), instance.getVKInstance(), &_vkSurface ) )
   {
      const char* error = SDL_GetError();
      CYDASSERT( !error );
   }
}

cyd::Surface::~Surface()
{
   vkDestroySurfaceKHR( _instance.getVKInstance(), _vkSurface, nullptr );
   _vkSurface = nullptr;
}
