#pragma once

#include <Common/Include.h>

#include <Applications/Application.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This Vulkan sandbox is used as a template to experiment
*/
namespace CYD
{
class VKSandbox final : public Application
{
  public:
   VKSandbox( uint32_t width, uint32_t height, const std::string& title );
   NON_COPIABLE( VKSandbox )
   ~VKSandbox() override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;
};
}
