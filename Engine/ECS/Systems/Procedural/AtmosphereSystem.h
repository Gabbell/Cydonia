#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Procedural/AtmosphereComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This system is used to generate a physically-based atmosphere that includes customizable Rayleigh and
Mie scattering factors to generate any kind of planetary atmosphere. 

Does not currently implement aerial perspective LUT and views from space

Hillaire, S. (2020). A scalable and production ready sky and atmosphere rendering technique.
Computer Graphics Forum, 39(4), 13–22. https://doi.org/10.1111/cgf.14050

https://sebh.github.io/publications/egsr2020.pdf
https://github.com/sebh/UnrealEngineSkyAtmosphere
https://www.shadertoy.com/view/slSXRW
*/
namespace CYD
{
class AtmosphereSystem final : public CommonSystem<RenderableComponent, AtmosphereComponent>
{
  public:
   AtmosphereSystem() = default;
   NON_COPIABLE( AtmosphereSystem );
   virtual ~AtmosphereSystem() = default;

   void tick( double deltaS ) override;
};
}
