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

class GBufferSystem final : public RenderSystem
{
  public:
   GBufferSystem() = delete;
   GBufferSystem( const MaterialCache& materials ) : RenderSystem( materials ) {}
   NON_COPIABLE( GBufferSystem );
   virtual ~GBufferSystem() = default;

   void sort() override;

   void tick( double deltaS ) override;
};
}
