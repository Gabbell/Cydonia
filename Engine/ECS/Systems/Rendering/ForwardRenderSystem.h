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

class ForwardRenderSystem final : public CommonSystem<
                                      RenderableComponent,
                                      TransformComponent,
                                      MeshComponent,
                                      MaterialComponent>
{
  public:
   ForwardRenderSystem() = delete;
   ForwardRenderSystem( const MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( ForwardRenderSystem );
   virtual ~ForwardRenderSystem() = default;

   void tick( double deltaS ) override;

  protected:
   bool _compareEntities( const EntityEntry& first, const EntityEntry& second ) override;

  private:
   const MaterialCache& m_materials;
};
}
