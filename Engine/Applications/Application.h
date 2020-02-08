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
}

// =================================================================================================
// Definition
// =================================================================================================
namespace cyd
{
class Application
{
  public:
   Application();
   NON_COPIABLE( Application )
   virtual ~Application();

   virtual bool init( uint32_t width, uint32_t height, const std::string& title );

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
