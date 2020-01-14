#include <Applications/Application.h>

#include <Window/GLFWWindow.h>

#include <HID/InputInterpreter.h>

#include <chrono>
#include <memory>

namespace cyd
{
Application::Application() = default;

bool Application::init( uint32_t width, uint32_t height, const std::string& title )
{
   m_window           = std::make_unique<Window>();
   m_inputInterpreter = std::make_unique<InputInterpreter>();

   bool success = true;
   success &= m_window->init( width, height, title );
   success &= m_inputInterpreter->init( *m_window );

   return success;
}

void Application::startLoop()
{
   static auto start = std::chrono::high_resolution_clock::now();

   preLoop();

   m_running = true;
   while( m_running )  // Main loop
   {
      // Calculate delta time between frames
      const std::chrono::duration<double> deltaMs =
          std::chrono::high_resolution_clock::now() - start;

      // Reset clock
      start = std::chrono::high_resolution_clock::now();

      // TODO Replace with input system
      m_inputInterpreter->tick();

      // User overloaded tick
      tick( deltaMs.count() );

      // Determine if the main window was asked to be closed
      m_running = m_window->isRunning();
   }

   postLoop();
}

void Application::preLoop() {}
void Application::tick( double /*deltaMs*/ ) {}
void Application::postLoop() {}

Application::~Application() = default;
}
