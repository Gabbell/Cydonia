#pragma once

#include <Applications/Application.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class VKApplication : public Application
{
  public:
   VKApplication() = delete;
   VKApplication( uint32_t width, uint32_t height, const std::string& title );
   NON_COPIABLE( VKApplication );
   virtual ~VKApplication();
};
}
