#include <Core/Application.h>

#include <Core/Common/Assert.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Instance.h>
#include <Core/Graphics/Surface.h>
#include <Core/Graphics/DeviceManager.h>

#include <SDL2/SDL.h>

cyd::Application::Application( uint32_t width, uint32_t height, const std::string& title )
    : _running( true )
{
   _window   = std::make_unique<Window>( width, height, title );
   _instance = std::make_unique<Instance>( *_window );
   _surface  = std::make_unique<Surface>( *_instance , *_window );
   _dm       = std::make_unique<DeviceManager>( *_instance, *_window, *_surface );

   const Swapchain* sc = _dm->getMainSwapchain();
}

void cyd::Application::startLoop()
{
   preLoop();
   while( _running )  // Main loop
   {
      tick();
   }
   postLoop();
}

void cyd::Application::preLoop() {}
void cyd::Application::tick()
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
void cyd::Application::drawFrame() {}
void cyd::Application::postLoop() {}

cyd::Application::~Application() {}
