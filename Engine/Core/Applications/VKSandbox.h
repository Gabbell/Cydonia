#pragma once

#include <Core/Applications/Application.h>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Buffer;
class Texture;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VKSandbox final : public Application
{
  public:
   VKSandbox() = delete;
   VKSandbox( uint32_t width, uint32_t height );
   VKSandbox( const VKSandbox& other )     = delete;
   VKSandbox( VKSandbox&& other ) noexcept = delete;
   VKSandbox& operator=( const VKSandbox& other ) = delete;
   VKSandbox& operator=( VKSandbox&& other ) noexcept = delete;
   ~VKSandbox();

  protected:
   virtual void preLoop() override;  // Executed before the application enters the main loop
   virtual void drawFrame( double deltaTime ) override;  // Used to draw one frame

  private:
   std::shared_ptr<cyd::Buffer> _vertexBuffer = nullptr;
   std::shared_ptr<cyd::Buffer> _uboBuffer    = nullptr;
   std::shared_ptr<cyd::Texture> _texture     = nullptr;
};
}
