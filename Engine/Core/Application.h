#pragma once

#include <cstdint>
#include <memory>
#include <string>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Window;
class Instance;
class Surface;
class DeviceHerder;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Application
{
  public:
   Application( uint32_t width, uint32_t height, const std::string& title );
   Application( const Application& other )     = delete;
   Application( Application&& other ) noexcept = delete;
   Application& operator=( const Application& other ) = delete;
   Application& operator=( Application&& other ) noexcept = delete;
   ~Application();

   void startLoop();

  protected:
   virtual void preLoop();                 // Executed before the application enters the main loop
   virtual void tick( double deltaTime );  // Executed as fast as possible
   virtual void drawFrame( double deltaTime );  // Used to draw one frame
   virtual void postLoop();  // Executed when the application comes out of the main loop

  private:
   std::unique_ptr<Window> _window;
   std::unique_ptr<Instance> _instance;
   std::unique_ptr<Surface> _surface;
   std::unique_ptr<DeviceHerder> _dh;

   bool _running;
};
}