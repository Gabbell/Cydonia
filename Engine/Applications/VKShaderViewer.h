#pragma once

#include <Common/Include.h>

#include <Applications/Application.h>

#include <string>

// ================================================================================================
// Definition
// ================================================================================================
/*
This Vulkan shader viewer template is used to visualize simple fragment-only shaders
*/
namespace CYD
{
class VKShaderViewer final : public Application
{
  public:
   VKShaderViewer( uint32_t width, uint32_t height, const std::string& shaderName );
   NON_COPIABLE( VKShaderViewer )
   ~VKShaderViewer() override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;

  private:
   const std::string m_fragShader;
};
}
