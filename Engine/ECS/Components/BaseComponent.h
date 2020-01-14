#pragma once

#include <Common/Include.h>

namespace cyd
{
class BaseComponent
{
  public:
   COPIABLE( BaseComponent );
   virtual ~BaseComponent() = default;

   virtual bool init()   = 0;
   virtual void uninit() = 0;

  protected:
   BaseComponent() = default;
};
}
