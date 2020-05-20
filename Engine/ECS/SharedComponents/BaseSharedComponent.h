#pragma once

#include <Common/Include.h>

namespace CYD
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
