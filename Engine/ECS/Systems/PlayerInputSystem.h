#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/FreeControllerComponent.h>

namespace cyd
{
class PlayerInputSystem final : public CommonSystem<TransformComponent, FreeControllerComponent>
{
   explicit PlayerInputSystem( const EntityManager& entityManager ) : CommonSystem( entityManager )
   {
   }
   NON_COPIABLE( PlayerInputSystem );
   virtual ~PlayerInputSystem() = default;

   bool init() override;
   void tick( double deltaMs ) override;
};
}
