#include <ECS/EntityManager.h>

#include <ECS/Systems/BaseSystem.h>

#include <ECS/Systems/RenderSystem.h>

namespace cyd
{
bool EntityManager::init()
{
   m_systems.push_back( new RenderSystem( *this ) );
   return true;
}

void EntityManager::tick( double deltaMs )
{
   for( auto& system : m_systems )
   {
      system->tick( deltaMs );
   }
}

EntityHandle EntityManager::addEntity()
{
   // Building entity
   const EntityHandle handle = m_entities.size();
   Entity entity( handle );

   // TODO Add archetype components to entity

   for( auto& system : m_systems )  // Notify each system that an entity was created
   {
      system->onEntityCreate( entity );
   }

   return handle;
}

void EntityManager::removeEntity( EntityHandle )
{
   //
}

EntityManager::~EntityManager()
{
   for( auto& system : m_systems )
   {
      delete system;
   }
}
}
