#include <ECS/Systems/Behaviour/EntityFollowSystem.h>

#include <ECS/EntityManager.h>

namespace CYD
{
void EntityFollowSystem::tick( double /*deltaS*/ )
{
   for( const auto& entityEntry : m_entities )
   {
      const EntityFollowComponent& follow = *std::get<EntityFollowComponent*>( entityEntry.arch );
      TransformComponent& transform       = *std::get<TransformComponent*>( entityEntry.arch );

      // Getting the position of the followed entity and setting the current entity's position to it
      const Entity* followedEntity = m_ecs->getEntity( follow.entity );
      if( followedEntity )
      {
         // TODO Is this OK? Probably, because we cannot modify the entity.
         const TransformComponent* otherTransform =
             followedEntity->getComponent<TransformComponent>();

         if( otherTransform )
         {
            transform.position = otherTransform->position;
         }
      }
   }
}
}
