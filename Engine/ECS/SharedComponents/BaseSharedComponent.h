#pragma once

#include <Common/Include.h>

namespace cyd
{
class BaseSharedComponent
{
  public:
   NON_COPIABLE( BaseSharedComponent )
   virtual ~BaseSharedComponent() = default;

  protected:
   BaseSharedComponent() = default;
};
}
