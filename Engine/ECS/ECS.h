#pragma once

#include <Common/Assert.h>

#include <ECS/Entities/Entity.h>
#include <ECS/Components/ComponentPool.h>
#include <ECS/Systems/CommonSystem.h>

#include <array>
#include <unordered_map>
#include <vector>

namespace cyd::ECS
{
namespace detail
{
using Entities   = std::unordered_map<EntityHandle, Entity>;
using Components = std::array<BaseComponentPool*, static_cast<size_t>( ComponentType::COUNT )>;
using Systems    = std::vector<BaseSystem*>;

// All entities currently managed by the manager (all entities in the world)
inline Entities entities;

// Pools of all components. Index is component type.
inline Components components;

// All currently running data transformation systems
inline Systems systems;
};

bool Initialize();
void Uninitialize();

void Tick( double deltaMs );

EntityHandle CreateEntity();

void RemoveEntity( EntityHandle handle );

template <class Component, typename... Args>
void Assign( EntityHandle handle, Args&&... args )
{
   static_assert( std::is_base_of_v<BaseComponent, Component>, "Assigning an invalid component" );

   auto it = detail::entities.find( handle );
   if( it == detail::entities.end() )
   {
      CYDASSERT( !"ECS: Could not find entity" );
      return;
   }

   // Fetching the adequate component pool
   ComponentPool<Component>*& pPool =
       (ComponentPool<Component>*&)detail::components[static_cast<size_t>( Component::TYPE )];
   if( !pPool )
   {
      // Pool has never been created, create it
      pPool = new ComponentPool<Component>();
   }

   Component* pComponent = pPool->acquireComponent( args... );
   it->second.addComponent<Component>( pComponent );

   // Notify systems that an entity was assigned a component
   for( auto& system : detail::systems )
   {
      system->onEntityAssigned( it->second );
   }
}

template <class Component>
void Unassign( EntityHandle handle )
{
   // Deallocate component from component pool
   // Remove it from the entity
   // Notify systems that an entity was unassigned a component
}

template <class System>
void AddSystem()
{
   static_assert( std::is_base_of_v<BaseSystem, System>, "Adding an invalid system" );
   detail::systems.push_back( new System() );
}
};
