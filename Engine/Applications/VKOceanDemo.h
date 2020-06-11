#pragma once

#include <Common/Include.h>

#include <Applications/Application.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This Vulkan demo is for an FFT ocean render
*/
namespace CYD
{
class VKOceanDemo final : public Application
{
  public:
   VKOceanDemo( uint32_t width, uint32_t height, const char* title );
   NON_COPIABLE( VKOceanDemo )
   ~VKOceanDemo() override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;
};
}
