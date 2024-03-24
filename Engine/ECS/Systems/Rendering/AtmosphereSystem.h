#pragma once

#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Procedural/AtmosphereComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class AtmosphereSystem final : public CommonSystem<RenderableComponent, AtmosphereComponent>
{
  public:
   AtmosphereSystem() = default;
   NON_COPIABLE( AtmosphereSystem );
   virtual ~AtmosphereSystem() = default;

   void tick( double deltaS ) override;
};
}
