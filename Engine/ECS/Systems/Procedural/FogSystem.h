#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Procedural/FogComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
A basic distance and exponential height fog system using a fullscreen compute pass
*/
namespace CYD
{
class FogSystem final : public CommonSystem<RenderableComponent, FogComponent>
{
  public:
   FogSystem() = default;
   NON_COPIABLE( FogSystem );
   virtual ~FogSystem() = default;

   void tick( double deltaS ) override;
};
}
