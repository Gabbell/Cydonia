#include <ECS/Systems/Behaviour/EntityFollowSystem.h>

#include <ECS/EntityManager.h>

namespace CYD
{
void EntityFollowSystem::tick( double /*deltaS*/ )
{
   for( const auto& compPair : m_components )
   {
      const EntityFollowComponent& follow = *std::get<EntityFollowComponent*>( compPair.second );
      TransformComponent& transform       = *std::get<TransformComponent*>( compPair.second );

      // Getting the position of the followed entity and setting the current entity's position to it
      const Entity* followedEntity = ECS::GetEntity( follow.entity );
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
