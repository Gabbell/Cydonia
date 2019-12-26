#pragma once

#include <Common/Include.h>

#include <Applications/VKApplication.h>

#include <string>

// ================================================================================================
// Forward
// ================================================================================================
namespace vk
{
class Buffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VKShaderViewer final : public VKApplication
{
  public:
   VKShaderViewer() = delete;
   VKShaderViewer(
       uint32_t width,
       uint32_t height,
       const std::string& vertShader,
       const std::string& fragShader );
   NON_COPIABLE( VKShaderViewer );
   ~VKShaderViewer();

  protected:
   virtual void preLoop() override;  // Executed before the application enters the main loop
   virtual void drawFrame( double deltaMs ) override;  // Used to draw one frame

  private:
   const std::string m_vertShader;
   const std::string m_fragShader;
   std::shared_ptr<vk::Buffer> m_vertexBuffer = nullptr;
};
}
