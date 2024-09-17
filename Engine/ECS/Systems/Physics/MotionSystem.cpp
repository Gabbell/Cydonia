#include <ECS/Systems/Physics/MotionSystem.h>

#include <Graphics/Utility/Transforms.h>

namespace CYD
{
void MotionSystem::tick( double deltaS )
{
   for( const auto& entityEntry : m_entities )
   {
      TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      const MotionComponent& motion = GetComponent<MotionComponent>( entityEntry );

      Transform::Translate( transform.position, motion.velocity * static_cast<float>( deltaS ) );
   }
}
}