#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/AtmosphereComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class AtmosphereSystem final : public CommonSystem<AtmosphereComponent>
{
  public:
   AtmosphereSystem() = default;
   NON_COPIABLE( AtmosphereSystem );
   virtual ~AtmosphereSystem() = default;

   void tick( double deltaS ) override;
};
}
