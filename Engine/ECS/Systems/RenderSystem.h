#pragma once

#include <Common/Include.h>

#include <ECS/Systems/CommonSystem.h>

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
   explicit RenderSystem( const EntityManager& entityManager ) : CommonSystem( entityManager ) {}
   NON_COPIABLE( RenderSystem );
   virtual ~RenderSystem();

   bool init() override;
   void tick( double deltaMs ) override;

  private:
   unsigned char* data = nullptr;
};
}
