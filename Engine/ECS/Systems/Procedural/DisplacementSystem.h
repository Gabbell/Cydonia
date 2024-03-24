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

class DisplacementUpdateSystem final
    : public CommonSystem<RenderableComponent, DisplacementComponent, MaterialComponent>
{
  public:
   DisplacementUpdateSystem() = delete;
   DisplacementUpdateSystem( MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( DisplacementUpdateSystem );
   virtual ~DisplacementUpdateSystem() = default;

   void tick( double deltaS ) override;

  private:
   MaterialCache& m_materials;
};
}