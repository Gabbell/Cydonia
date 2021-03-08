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
class PBRRenderSystem final
    : public CommonSystem<TransformComponent, MeshComponent, MaterialComponent, RenderableComponent>
{
  public:
   PBRRenderSystem() = default;
   NON_COPIABLE( PBRRenderSystem );
   virtual ~PBRRenderSystem() = default;

   void tick( double deltaS ) override;
   bool compareEntities( const EntityEntry& first, const EntityEntry& second ) override;
};
}
