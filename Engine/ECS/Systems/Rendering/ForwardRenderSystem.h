#pragma once

#include <ECS/Systems/Rendering/RenderSystem.h>

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

class ForwardRenderSystem final : public RenderSystem
{
  public:
   ForwardRenderSystem() = delete;
   ForwardRenderSystem( const MaterialCache& materials ) : RenderSystem( materials ) {}
   NON_COPIABLE( ForwardRenderSystem );
   virtual ~ForwardRenderSystem() = default;

   void sort() override;

   void tick( double deltaS ) override;
};
}
