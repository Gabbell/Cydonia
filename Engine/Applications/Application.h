#pragma once

#include <Common/Include.h>

#include <cstdint>
#include <memory>
#include <string>

// =================================================================================================
// Forwards
// =================================================================================================
namespace cyd
{
class Window;
class InputInterpreter;
class Scene;
}

// =================================================================================================
// Definition
// =================================================================================================
namespace cyd
{
class Application
{
  public:
   Application() = delete;
   Application( uint32_t width, uint32_t height, const std::string& title );
   NON_COPIABLE( Application );
   virtual ~Application();

   void startLoop();

  protected:
   virtual void preLoop();                 // Executed before the application enters the main loop
   virtual void tick( double deltaTime );  // Executed as fast as possible
   virtual void drawNextFrame( double deltaTime );  // Used to draw one frame
   virtual void postLoop();  // Executed when the application comes out of the main loop

   // Systems
   std::unique_ptr<Window> _window;
   std::unique_ptr<InputInterpreter> _inputInterpreter;
   std::unique_ptr<Scene> _scene;

  private:
   bool _running;
};
}
