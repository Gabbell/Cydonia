#pragma once

#include <Core/Application.h>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Instance;
class Surface;
class DeviceHerder;
class Buffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VkApplication : public Application
{
  public:
   VkApplication() = delete;
   VkApplication( uint32_t width, uint32_t height, const std::string& title );
   VkApplication( const VkApplication& other )     = delete;
   VkApplication( VkApplication&& other ) noexcept = delete;
   VkApplication& operator=( const VkApplication& other ) = delete;
   VkApplication& operator=( VkApplication&& other ) noexcept = delete;
   ~VkApplication();

  protected:
   virtual void drawFrame( double deltaTime ) override;  // Used to draw one frame

  private:
   std::unique_ptr<Instance> _instance;
   std::unique_ptr<Surface> _surface;
   std::unique_ptr<DeviceHerder> _dh;

   std::shared_ptr<cyd::Buffer> _vertexBuffer = nullptr;
};
}
