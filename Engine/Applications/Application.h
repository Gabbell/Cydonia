#pragma once

#include <Common/Include.h>

#include <cstdint>
#include <memory>

// =================================================================================================
// Forwards
// =================================================================================================
namespace CYD
{
class Window;
}

// =================================================================================================
// Definition
// =================================================================================================
namespace CYD
{
class Application
{
  public:
   Application( uint32_t width, uint32_t height, const char* title );
   NON_COPIABLE( Application );
   virtual ~Application();

   void startLoop();

  protected:
   virtual void preLoop();              // Executed before the application enters the main loop
   virtual void tick( double deltaS );  // Executed as fast as possible
   virtual void postLoop();             // Executed when the application comes out of the main loop

   std::unique_ptr<Window> m_window;

  private:
   bool m_running = false;
};
}
