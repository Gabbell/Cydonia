#include <ECS/Systems/Physics/MotionSystem.h>

#include <Graphics/Utility/Transforms.h>

namespace CYD
{
void MotionSystem::tick( double deltaS )
{
   for( const auto& entityEntry : m_entities )
   {
      TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MotionComponent& motion = *std::get<MotionComponent*>( entityEntry.arch );

      Transform::Translate( transform.position, motion.velocity * static_cast<float>( deltaS ) );
   }
}
}