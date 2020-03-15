#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class RenderSystem final : public CommonSystem<TransformComponent, RenderableComponent>
{
  public:
   RenderSystem() = default;
   NON_COPIABLE( RenderSystem )
   virtual ~RenderSystem() = default;

   bool init() override;
   void uninit() override{};

   void tick( double deltaS ) override;
};
}
