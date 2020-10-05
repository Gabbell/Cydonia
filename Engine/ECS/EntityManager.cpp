#include <ECS/EntityManager.h>

#include <ECS/Components/BaseComponent.h>

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD::ECS
{
bool Initialize()
{
   // Initializing shared components
   detail::sharedComponents[(size_t)SharedComponentType::INPUT]  = new InputComponent();
   detail::sharedComponents[(size_t)SharedComponentType::CAMERA] = new CameraComponent();
   detail::sharedComponents[(size_t)SharedComponentType::SCENE]  = new SceneComponent();

   return true;
}

void Tick( double deltaS )
{
   // Ordered updating
   for( auto& system : detail::systems )
   {
      if( system->hasToTick() )  // Must have to not needlessly tick the systems
      {
         system->tick( deltaS );
      }
   }
}

EntityHandle CreateEntity()
{
   // Building entity
   static int uid            = 0;
   const EntityHandle handle = uid++;
   detail::entities[handle]  = Entity( handle );

   return handle;
}

const Entity* GetEntity( EntityHandle handle )
{
   const auto it = detail::entities.find( handle );
   if( it == detail::entities.end() )
   {
      CYDASSERT( !"Tried to get an entity that does not exist" );
      return nullptr;
   }

   return &it->second;
}

void RemoveEntity( EntityHandle handle )
{
   const auto it = detail::entities.find( handle );
   if( it == detail::entities.end() )
   {
      CYDASSERT( !"Tried to remove an entity that does not exist" );
      return;
   }

   const Entity& entity = it->second;

   // Deallocating components
   for( const auto& component : entity.getComponents() )
   {
      // Remove from pool
      detail::components[(size_t)component.first]->releaseComponent(
          component.second->getPoolIndex() );
   }

   // Notifying systems that an entity was removed
   for( auto& system : detail::systems )
   {
      system->onEntityUnassigned( entity );
   }

   detail::entities.erase( it );
}

void Uninitialize()
{
   for( auto& sharedComponent : detail::sharedComponents )
   {
      delete sharedComponent;
   }
   for( auto& componentPool : detail::components )
   {
      delete componentPool;
   }
   for( auto& system : detail::systems )
   {
      delete system;
   }
}
}
