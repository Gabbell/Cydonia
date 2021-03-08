#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/AtmosphereComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class AtmosphereRenderSystem final : public CommonSystem<RenderableComponent, AtmosphereComponent>
{
  public:
   AtmosphereRenderSystem() = default;
   NON_COPIABLE( AtmosphereRenderSystem );
   virtual ~AtmosphereRenderSystem() = default;

   void tick( double deltaS ) override;
};
}
