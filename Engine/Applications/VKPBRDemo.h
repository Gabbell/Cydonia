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
class VKPBRDemo final : public Application
{
  public:
   VKPBRDemo( uint32_t width, uint32_t height, const char* title );
   NON_COPIABLE( VKPBRDemo )
   ~VKPBRDemo() override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;
};
}
