#include <ECS/Systems/MovementSystem.h>

namespace cyd
{
void MovementSystem::tick( double deltaS )
{
   for( const auto& compPair : m_components )
   {
      TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );
      const MotionComponent& motion = *std::get<MotionComponent*>( compPair.second );

      transform.position += ( motion.velocity * static_cast<float>( deltaS ) );
   }
}
}