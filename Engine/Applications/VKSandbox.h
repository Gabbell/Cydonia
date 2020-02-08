#pragma once

#include <Common/Include.h>

#include <Applications/Application.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VKSandbox final : public Application
{
  public:
   VKSandbox();
   NON_COPIABLE( VKSandbox )
   ~VKSandbox() override;

   bool init( uint32_t width, uint32_t height, const std::string& title ) override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;
   void postLoop() override;
};
}
