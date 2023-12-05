#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Procedural/DisplacementComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class DisplacementSystem final
    : public CommonSystem<RenderableComponent, TransformComponent, DisplacementComponent, MaterialComponent>
{
  public:
   DisplacementSystem() = delete;
   DisplacementSystem( MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( DisplacementSystem );
   virtual ~DisplacementSystem() = default;

   void tick( double deltaS ) override;

  private:
   MaterialCache& m_materials;
};
}