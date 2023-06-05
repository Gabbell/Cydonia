#include <ECS/EntityManager.h>

#include <ECS/Components/BaseComponent.h>

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
EntityManager::EntityManager()
{
   // Initializing shared components
   m_sharedComponents[(size_t)SharedComponentType::INPUT] = new InputComponent();
   m_sharedComponents[(size_t)SharedComponentType::SCENE] = new SceneComponent();
}

EntityManager::~EntityManager()
{
   for( auto& sharedComponent : m_sharedComponents )
   {
      delete sharedComponent;
   }
   for( auto& componentPool : m_componentPools )
   {
      delete componentPool;
   }
   for( auto& system : m_systems )
   {
      delete system;
   }
}

void EntityManager::tick( double deltaS )
{
   // Ordered updating
   for( auto& system : m_systems )
   {
      if( system->hasToTick() )  // Must have to not needlessly tick the systems
      {
         system->tick( deltaS );
      }
   }
}

EntityHandle EntityManager::createEntity( std::string_view name )
{
   // Building entity
   static int uid            = 0;
   const EntityHandle handle = uid++;
   m_entities[handle]        = Entity( handle, name );

   return handle;
}

const Entity* EntityManager::getEntity( EntityHandle handle ) const
{
   const auto it = m_entities.find( handle );
   if( it == m_entities.end() )
   {
      CYDASSERT( !"Tried to get an entity that does not exist" );
      return nullptr;
   }

   return &it->second;
}

void EntityManager::removeEntity( EntityHandle handle )
{
   const auto it = m_entities.find( handle );
   if( it == m_entities.end() )
   {
      CYDASSERT( !"Tried to remove an entity that does not exist" );
      return;
   }

   const Entity& entity = it->second;

   // Deallocating components
   for( const auto& component : entity.getComponents() )
   {
      // Remove from pool
      m_componentPools[(size_t)component.first]->releaseComponent(
          component.second->getPoolIndex() );
   }

   // Notifying systems that an entity was removed
   for( auto& system : m_systems )
   {
      system->onEntityUnassigned( entity );
   }

   m_entities.erase( it );
}
}
