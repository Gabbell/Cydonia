#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/SkyboxComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class SkyboxSystem final
    : public CommonSystem<TransformComponent, MeshComponent, RenderableComponent, SkyboxComponent>
{
  public:
   SkyboxSystem() = default;
   NON_COPIABLE( SkyboxSystem );
   virtual ~SkyboxSystem() = default;

   void tick( double deltaS ) override;
};
}
