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
class PhongRenderSystem final : public CommonSystem<TransformComponent, RenderableComponent>
{
  public:
   PhongRenderSystem() = default;
   NON_COPIABLE( PhongRenderSystem )
   virtual ~PhongRenderSystem() = default;

   bool init() override;
   void uninit() override;
   
   void tick( double deltaS ) override;
};
}
