#include "App.h"

#include "Instance.h"
#include "Window.h"
#include "DeviceManager.h"

#include <SDL2/SDL.h>

cyd::App::App( uint32_t width, uint32_t height, const std::string& title ) : _running( true )
{
   // Creating window
   _window = std::make_unique<Window>( width, height, title );

   // We need the window to fetch the required extensions when creating the instance
   _instance = std::make_unique<Instance>( _window.get() );

   // We need the instance to create the surface which is per window
   _window->createSurface( _instance.get() );

   // We need the surface to create the swapchain
   //_swapchain = std::make_unique<Swapchain>(_window.get());

   // Creating device manager
   _deviceManager = std::make_unique<DeviceManager>( _instance.get() );
}

void cyd::App::startLoop()
{
   // Main loop
   while( _running )
   {
      // TODO Make an input manager
      SDL_Event event;
      while( SDL_PollEvent( &event ) )
      {
         switch( event.type )
         {
            case SDL_KEYDOWN:
               if( event.key.keysym.sym == SDLK_ESCAPE )
               {
                  _running = false;
               }
               break;

            default:
               break;
         }
      }
   }
}

cyd::App::~App() {}
