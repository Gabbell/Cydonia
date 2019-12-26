#include <Applications/Application.h>

#include <Window/GLFWWindow.h>

#include <HID/InputInterpreter.h>

#include <Graphics/Scene/SceneContext.h>

#include <ECS/EntityManager.h>

#include <chrono>
#include <memory>

namespace cyd
{
bool Application::init( uint32_t width, uint32_t height, const std::string& title )
{
   m_window           = std::make_unique<Window>();
   m_inputInterpreter = std::make_unique<InputInterpreter>();
   m_sceneContext     = std::make_unique<SceneContext>();

   m_entityManager = std::make_unique<EntityManager>();

   bool success = true;
   success &= m_window->init( width, height, title );
   success &= m_inputInterpreter->init( *m_window );
   success &= m_entityManager->init();

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

      // Systems Tick
      m_entityManager->tick( deltaMs.count() );

      // TODO Replace with input system
      m_inputInterpreter->tick();

      // User overloaded tick
      tick( deltaMs.count() );

      // Draw the next frame
      drawFrame( deltaMs.count() );

      // Determine if the main window was asked to be closed
      m_running = m_window->isRunning();
   }

   postLoop();
}

void Application::preLoop() {}
void Application::tick( double /*deltaMs*/ ) {}
void Application::drawFrame( double /*deltaMs*/ ) {}
void Application::postLoop() {}

Application::~Application() = default;
}
