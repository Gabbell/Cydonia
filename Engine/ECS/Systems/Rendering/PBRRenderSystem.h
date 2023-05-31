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

class PBRRenderSystem final
    : public CommonSystem<TransformComponent, MeshComponent, MaterialComponent, RenderableComponent>
{
  public:
   PBRRenderSystem() = delete;
   PBRRenderSystem( const MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( PBRRenderSystem );
   virtual ~PBRRenderSystem() = default;

   void tick( double deltaS ) override;

  protected:
   bool _compareEntities( const EntityEntry& first, const EntityEntry& second ) override;

  private:
   const MaterialCache& m_materials;
};
}
