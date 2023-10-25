#pragma once

#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Procedural/AtmosphereComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class AtmosphereRenderSystem final : public CommonSystem<AtmosphereComponent>
{
  public:
   AtmosphereRenderSystem() = default;
   NON_COPIABLE( AtmosphereRenderSystem );
   virtual ~AtmosphereRenderSystem() = default;

   void tick( double deltaS ) override;
};
}
