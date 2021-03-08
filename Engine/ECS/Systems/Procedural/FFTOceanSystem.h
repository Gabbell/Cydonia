#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This system is used to generate an displacement field for entities that have an FFTOceanComponent.
*/
namespace CYD
{
class FFTOceanSystem final : public CommonSystem<MaterialComponent, FFTOceanComponent>
{
  public:
   FFTOceanSystem() = default;
   NON_COPIABLE( FFTOceanSystem );
   virtual ~FFTOceanSystem() = default;

   void tick( double deltaS ) override;
};
}
