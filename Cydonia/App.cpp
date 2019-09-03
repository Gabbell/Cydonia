// Enable the WSI extensions
#if defined( __linux__ )
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include "App.h"

#include "Instance.h"
#include "Window.h"
#include "DeviceManager.h"

#include <SDL2/SDL.h>

cyd::App::App( uint32_t width, uint32_t height, const std::string& title ) : _running( true )
{
   // Creating instance
   _instance = std::make_unique<Instance>();

   // Creating window
   _window = std::make_unique<Window>( width, height, title, _instance.get() );

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