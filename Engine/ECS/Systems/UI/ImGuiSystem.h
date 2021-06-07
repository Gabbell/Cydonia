#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class ImGuiSystem final : public CommonSystem<>
{
  public:
   ImGuiSystem() = default;
   NON_COPIABLE( ImGuiSystem );
   virtual ~ImGuiSystem() = default;

   void tick( double deltaS ) override;
};
}
