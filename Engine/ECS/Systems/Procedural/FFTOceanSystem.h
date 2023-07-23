#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This system is used to generate a displacement field for entities that have an FFTOceanComponent.
Sources:

Flügge, F. (2017). Realtime GPGPU FFT Ocean Water Simulation [Research Project Thesis, Hamburg
University of Technology]. https://doi.org/10.15480/882.1436

Tessendorf, J. (2004). Simulating Ocean Water.
*/
namespace CYD
{
class MaterialCache;

class FFTOceanSystem final : public CommonSystem<MaterialComponent, FFTOceanComponent>
{
  public:
   FFTOceanSystem() = delete;
   FFTOceanSystem( MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( FFTOceanSystem );
   virtual ~FFTOceanSystem() = default;

   void tick( double deltaS ) override;

  private:
   MaterialCache& m_materials;
};
}
