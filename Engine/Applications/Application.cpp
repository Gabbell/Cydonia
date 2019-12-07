#include <Applications/Application.h>

#include <Window/GLFWWindow.h>

#include <HID/InputInterpreter.h>

#include <Graphics/Scene/Scene.h>

#include <chrono>
#include <memory>

namespace cyd
{
Application::Application( uint32_t width, uint32_t height, const std::string& title )
    : _running( true )
{
   _window           = std::make_unique<Window>( width, height, title );
   _inputInterpreter = std::make_unique<InputInterpreter>( *_window );
   _scene            = std::make_unique<Scene>();
}

void Application::startLoop()
{
   preLoop();

   static auto start = std::chrono::high_resolution_clock::now();
   while( _running )  // Main loop
   {
      // Calculate delta time between frames
      const std::chrono::duration<double> deltaTime =
          std::chrono::high_resolution_clock::now() - start;

      // Reset clock
      start = std::chrono::high_resolution_clock::now();

      // Systems Tick
      _inputInterpreter->tick();

      tick( deltaTime.count() );
      drawNextFrame( deltaTime.count() );

      // Determine if the main window was asked to be closed
      _running = _window->isRunning();
   }

   postLoop();
}

void Application::preLoop() {}
void Application::tick( double /*deltaTime*/ ) {}

void Application::drawNextFrame( double /*deltaTime*/ ) {}
void Application::postLoop() {}

Application::~Application() {}
}