#pragma once

#include <Common/Include.h>

#include <Applications/VKApplication.h>

#include <Handles/Handle.h>

#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class FreeCameraController;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VKSandbox final : public VKApplication
{
  public:
   VKSandbox() = delete;
   VKSandbox( uint32_t width, uint32_t height );
   NON_COPIABLE( VKSandbox );
   virtual ~VKSandbox();

  protected:
   void preLoop() override;  // Executed before the application enters the main loop
   void drawNextFrame( double deltaTime ) override;  // Used to draw one frame
   void postLoop() override;

  private:
   std::unique_ptr<FreeCameraController> _controller = nullptr;

   VertexBufferHandle _vertexBuffer;
   IndexBufferHandle _indexBuffer;
   UniformBufferHandle _uboBuffer;
   TextureHandle _texture;
};
}
