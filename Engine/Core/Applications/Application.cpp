#include <Core/Applications/Application.h>

#include <Core/Window/Window.h>

// This might eventually go into a VKApplication.cpp
#include <Core/Graphics/Vulkan/Instance.h>
#include <Core/Graphics/Vulkan/Surface.h>
#include <Core/Graphics/Vulkan/DeviceHerder.h>
//

#include <SDL2/SDL.h>

#include <chrono>
#include <memory>

cyd::Application::Application( uint32_t width, uint32_t height, const std::string& title )
    : _running( true )
{
   _window   = std::make_unique<Window>( width, height, title );
   _instance = std::make_unique<Instance>( *_window );
   _surface  = std::make_unique<Surface>( *_instance, *_window );
   _dh       = std::make_unique<DeviceHerder>( *_instance, *_window, *_surface );
}

void cyd::Application::startLoop()
{
   static auto start = std::chrono::high_resolution_clock::now();

   preLoop();
   while( _running )  // Main loop
   {
      // Calculate delta time
      const std::chrono::duration<double> deltaTime =
          std::chrono::high_resolution_clock::now() - start;

      // Reset clock
      start = std::chrono::high_resolution_clock::now();

      tick( deltaTime.count() );
      drawFrame( deltaTime.count() );
   }
   postLoop();
}

void cyd::Application::preLoop() {}
void cyd::Application::tick( double deltaTime )
{
   // TODO Make an input/event manager

   // TODO There is currently a bug in SDL 2.0.9 where this stalls every 3000ms
   // https://bugzilla.libsdl.org/show_bug.cgi?id=4417

   //SDL_Event event;
   //while( SDL_PollEvent( &event ) )
   //{
   //   switch( event.type )
   //   {
   //      case SDL_KEYDOWN:
   //         if( event.key.keysym.sym == SDLK_ESCAPE )
   //         {
   //            _running = false;
   //         }
   //         break;

   //      default:
   //         break;
   //   }
   //}
}

void cyd::Application::drawFrame( double deltaTime ) {}
void cyd::Application::postLoop() {}

cyd::Application::~Application() {}
