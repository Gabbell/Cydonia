#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class ProceduralDisplacementSystem final
    : public CommonSystem<ProceduralDisplacementComponent, MaterialComponent>
{
  public:
   ProceduralDisplacementSystem() = delete;
   ProceduralDisplacementSystem( MaterialCache& materials );
   NON_COPIABLE( ProceduralDisplacementSystem );
   virtual ~ProceduralDisplacementSystem();

   void tick( double deltaS ) override;

  private:
   MaterialCache& m_materials;
};
}
