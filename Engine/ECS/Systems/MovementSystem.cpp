#include <ECS/Systems/MovementSystem.h>

namespace cyd
{
void MovementSystem::tick( double deltaS )
{
   for( const auto& archPair : m_archetypes )
   {
      TransformComponent& transform = *std::get<TransformComponent*>( archPair.second );
      const MotionComponent& motion = *std::get<MotionComponent*>( archPair.second );

      transform.position *= ( motion.velocity * static_cast<float>( deltaS ) );
   }
}
}