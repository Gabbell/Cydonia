#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

namespace CYD
{
class OptionsComponent final : public BaseSharedComponent
{
  public:
   OptionsComponent() = default;
   NON_COPIABLE( OptionsComponent );
   virtual ~OptionsComponent() = default;

   static constexpr SharedComponentType TYPE = SharedComponentType::OPTIONS;

   bool useVolumeShadows : 1 = false;
};
}
