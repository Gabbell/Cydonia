#include "Window.h"

#include "Assert.h"
#include "Instance.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

cyd::Window::Window(
    uint32_t width,
    uint32_t height,
    const std::string& title,
    const Instance* instance )
    : _width( width ), _height( height )
{
   if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
   {
      CYDASSERT( SDL_GetError() );
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

   if( SDL_Vulkan_CreateSurface( _sdlWindow, instance->getVKInstance(), &_surface ) < 0 )
   {
      CYDASSERT( SDL_GetError() );
   }
}

cyd::Window::~Window()
{
   SDL_DestroyWindow( _sdlWindow );
   SDL_Quit();
}
