#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class RenderSystem final
    : public CommonSystem<TransformComponent, MeshComponent, RenderableComponent>
{
  public:
   RenderSystem() = default;
   NON_COPIABLE( RenderSystem )
   virtual ~RenderSystem() = default;

   bool init() override;
   void uninit() override;

   void tick( double deltaS ) override;

  private:
   BufferHandle viewProjectionBuffer;
   BufferHandle cameraPosBuffer;
   BufferHandle lightsBuffer;
};
}
