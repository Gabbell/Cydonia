#pragma once

#include <Common/Include.h>

#include <Application.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This Direct3D12 sandbox is used as a template to experiment
*/
namespace CYD
{
class D3D12Sandbox final : public Application
{
  public:
   D3D12Sandbox( uint32_t width, uint32_t height, const char* title );
   NON_COPIABLE( D3D12Sandbox );
   ~D3D12Sandbox() override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;

  private:
};
}
