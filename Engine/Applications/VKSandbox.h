#pragma once

#include <Common/Include.h>

#include <Applications/Application.h>

#include <Graphics/Handles/Handle.h>

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
class VKSandbox final : public Application
{
  public:
   VKSandbox();
   NON_COPIABLE( VKSandbox );
   ~VKSandbox() override;

   bool init( uint32_t width, uint32_t height, const std::string& title ) override;

  protected:
   void preLoop() override;  // Executed before the application enters the main loop
   void drawFrame( double deltaMs ) override;  // Used to draw one frame
   void postLoop() override;

  private:
   std::unique_ptr<FreeCameraController> m_controller = nullptr;

   VertexBufferHandle m_vertexBuffer;
   IndexBufferHandle m_indexBuffer;
   UniformBufferHandle m_uboBuffer;
   TextureHandle m_texture;
};
}
