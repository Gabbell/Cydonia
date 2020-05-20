#include <ECS/Systems/Physics/MovementSystem.h>

#include <Graphics/Transforms.h>

namespace CYD
{
void MovementSystem::tick( double deltaS )
{
   for( const auto& compPair : m_components )
   {
      TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );
      const MotionComponent& motion = *std::get<MotionComponent*>( compPair.second );

      Transform::translate( transform.position, motion.velocity * static_cast<float>( deltaS ) );
   }
}
}