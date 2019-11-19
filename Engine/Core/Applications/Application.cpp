#include <Core/Applications/Application.h>

#include <Core/Window/GLFWWindow.h>

#include <Core/Input/InputInterpreter.h>

#include <Core/Graphics/Scene/SceneContext.h>

// This might eventually go into a VKApplication.cpp
#include <Core/Graphics/Vulkan/Instance.h>
#include <Core/Graphics/Vulkan/Surface.h>
#include <Core/Graphics/Vulkan/DeviceHerder.h>
//

#include <GLFW/glfw3.h>

#include <chrono>
#include <memory>

cyd::Application::Application( uint32_t width, uint32_t height, const std::string& title )
    : _running( true )
{
   _window = std::make_unique<Window>( width, height, title );

   _instance = std::make_unique<Instance>( *_window );
   _surface  = std::make_unique<Surface>( *_instance, *_window );
   _dh       = std::make_unique<DeviceHerder>( *_instance, *_window, *_surface );

   _inputInterpreter = std::make_unique<InputInterpreter>( *_window );

   const Rectangle viewport = { { 0.0f, 0.0f }, { width, height } };
   _sceneContext            = std::make_unique<SceneContext>( viewport );
}

void cyd::Application::startLoop()
{
   preLoop();

   static auto start = std::chrono::high_resolution_clock::now();
   while( _running )  // Main loop
   {
      // Polling events and interpreting them
      _inputInterpreter->pollAndInterpret();

      // Calculate delta time
      const std::chrono::duration<double> deltaTime =
          std::chrono::high_resolution_clock::now() - start;

      // Reset clock
      start = std::chrono::high_resolution_clock::now();

      tick( deltaTime.count() );
      drawFrame( deltaTime.count() );

      // Determine if the main window was asked to be closed
      _running = !glfwWindowShouldClose( _window->getGLFWwindow() );
   }

   postLoop();
}

void cyd::Application::preLoop() {}
void cyd::Application::tick( double /*deltaTime*/ ) {}

void cyd::Application::drawFrame( double /*deltaTime*/ ) {}
void cyd::Application::postLoop() {}

cyd::Application::~Application() {}
