#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <Graphics/RenderGraph.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class ForwardRenderSystem final
    : public CommonSystem<TransformComponent, MeshComponent, RenderableComponent>
{
  public:
   ForwardRenderSystem();
   NON_COPIABLE( ForwardRenderSystem );
   virtual ~ForwardRenderSystem() = default;

   void tick( double deltaS ) override;

  private:
   RenderGraph _renderGraph;
};
}
