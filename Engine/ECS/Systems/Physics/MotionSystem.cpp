#include <ECS/Systems/Physics/MotionSystem.h>

#include <Graphics/Utility/Transforms.h>

namespace CYD
{
void MotionSystem::tick( double deltaS )
{
   for( const auto& compPair : m_components )
   {
      TransformComponent& transform = *std::get<TransformComponent*>( compPair.second );
      const MotionComponent& motion = *std::get<MotionComponent*>( compPair.second );

      Transform::Translate( transform.position, motion.velocity * static_cast<float>( deltaS ) );
   }
}
}