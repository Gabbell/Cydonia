#include <ECS/Systems/PlayerMoveSystem.h>

#include <glm/glm.hpp>

namespace cyd
{
void PlayerMoveSystem::tick( double /*deltaS*/ )
{
   for( const auto& archPair : m_archetypes )
   {
      const InputComponent& input = *std::get<InputComponent*>( archPair.second );
      MotionComponent& motion     = *std::get<MotionComponent*>( archPair.second );

      glm::vec3 velocity( 0.0f );
      if( input.goingForwards )
      {
         velocity.z -= InputComponent::MOVE_SPEED;
      }
      if( input.goingBackwards )
      {
         velocity.z += InputComponent::MOVE_SPEED;
      }
      if( input.goingRight )
      {
         velocity.x += InputComponent::MOVE_SPEED;
      }
      if( input.goingLeft )
      {
         velocity.x -= InputComponent::MOVE_SPEED;
      }

      if( velocity != glm::vec3( 0.0f ) )
      {
         motion.velocity = velocity;
      }
   }
}
}
