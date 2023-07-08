#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class ShadowMapSystem final
    : public CommonSystem<RenderableComponent, TransformComponent, MaterialComponent, MeshComponent>
{
  public:
   ShadowMapSystem() = delete;
   ShadowMapSystem( const MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( ShadowMapSystem );
   virtual ~ShadowMapSystem() = default;

   void tick( double deltaS ) override;

  private:
   const MaterialCache& m_materials;
};
}