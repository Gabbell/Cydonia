#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Procedural/NoiseComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class NoiseGenerationSystem final : public CommonSystem<NoiseComponent, MaterialComponent>
{
  public:
   NoiseGenerationSystem() = delete;
   NoiseGenerationSystem( MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( NoiseGenerationSystem );
   virtual ~NoiseGenerationSystem() = default;

   void tick( double deltaS ) override;

  private:
   MaterialCache& m_materials;
};
}
