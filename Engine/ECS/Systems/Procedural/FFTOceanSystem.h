#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This system is used to generate an ocean height field for entities that have an FFTOceanComponent.
*/
namespace CYD
{
class FFTOceanSystem final : public CommonSystem<RenderableComponent, FFTOceanComponent>
{
  public:
   FFTOceanSystem() = default;
   NON_COPIABLE( FFTOceanSystem )
   virtual ~FFTOceanSystem() = default;

   bool init() override;
   void uninit() override;

   void tick( double deltaS ) override;
};
}
