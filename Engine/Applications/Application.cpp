#include <Applications/Application.h>

#include <Window/GLFWWindow.h>

#include <chrono>
#include <memory>

namespace CYD
{
Application::Application( uint32_t width, uint32_t height, const std::string& title )
{
   m_window = std::make_unique<Window>();
   m_window->init( width, height, title );
}

void Application::startLoop()
{
   static auto start = std::chrono::high_resolution_clock::now();

   preLoop();

   m_running = true;
   while( m_running )  // Main loop
   {
      // Calculate delta time between frames
      const std::chrono::duration<double> deltaS =
          std::chrono::high_resolution_clock::now() - start;

      // Reset clock
      start = std::chrono::high_resolution_clock::now();

      // User overloaded tick
      tick( deltaS.count() );

      // Determine if the main window was asked to be closed
      m_running = m_window->isRunning();
   }

   postLoop();
}

void Application::preLoop() {}
void Application::tick( double /*deltaS*/ ) {}
void Application::postLoop() {}

Application::~Application() = default;
}
