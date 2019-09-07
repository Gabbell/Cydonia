#pragma once

#include <cstdint>
#include <string>
#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Window;
class Instance;
class DeviceManager;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class App
{
  public:
   App( uint32_t width, uint32_t height, const std::string& title );
   App( const App& other )     = delete;
   App( App&& other ) noexcept = delete;
   App& operator=( const App& other ) = delete;
   App& operator=( App&& other ) noexcept = delete;
   ~App();

   void startLoop();

  private:
   std::unique_ptr<Instance> _instance;
   std::unique_ptr<Window> _window;
   std::unique_ptr<DeviceManager> _deviceManager;

   bool _running;
};
}
