#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class DeferredRenderSystem final : public CommonSystem<RenderableComponent>
{
  public:
   DeferredRenderSystem() = default;
   NON_COPIABLE( DeferredRenderSystem );
   virtual ~DeferredRenderSystem() = default;

   void tick( double deltaS ) override;
};
}
