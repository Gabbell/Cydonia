#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/DeferredRenderableComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class ShadowMapSystem final
    : public CommonSystem<TransformComponent, DeferredRenderableComponent, MeshComponent>
{
  public:
   ShadowMapSystem() = default;
   NON_COPIABLE( ShadowMapSystem );
   virtual ~ShadowMapSystem() = default;

   void tick( double deltaS ) override;
};
}