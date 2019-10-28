#include <Core/Application.h>

#include <Core/Window/Window.h>

#include <SDL2/SDL.h>

#include <chrono>
#include <memory>

cyd::Application::Application( uint32_t width, uint32_t height, const std::string& title )
    : _running( true )
{
   _window = std::make_unique<Window>( width, height, title );
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

void cyd::Application::drawFrame( double deltaTime ) {}
void cyd::Application::postLoop() {}