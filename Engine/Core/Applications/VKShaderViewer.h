#pragma once

#include <Core/Applications/Application.h>

// ================================================================================================
// Forward
// ================================================================================================
namespace cyd
{
class Buffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VKShaderViewer final : public Application
{
  public:
   VKShaderViewer() = delete;
   VKShaderViewer(
       uint32_t width,
       uint32_t height,
       const std::string& vertShader,
       const std::string& fragShader );
   VKShaderViewer( const VKShaderViewer& other )     = delete;
   VKShaderViewer( VKShaderViewer&& other ) noexcept = delete;
   VKShaderViewer& operator=( const VKShaderViewer& other ) = delete;
   VKShaderViewer& operator=( VKShaderViewer&& other ) noexcept = delete;
   ~VKShaderViewer();

  protected:
   virtual void preLoop() override;  // Executed before the application enters the main loop
   virtual void drawFrame( double deltaTime ) override;  // Used to draw one frame

  private:
   const std::string _vertShader;
   const std::string _fragShader;
   std::shared_ptr<cyd::Buffer> _vertexBuffer = nullptr;
};
}
