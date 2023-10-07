#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class DeferredRenderSystem final : public CommonSystem<>
{
  public:
   DeferredRenderSystem() = default;
   NON_COPIABLE( DeferredRenderSystem );
   virtual ~DeferredRenderSystem() = default;

   bool hasToTick() const noexcept override { return true; }
   void tick( double deltaS ) override;
};
}
