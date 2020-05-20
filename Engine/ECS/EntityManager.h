#pragma once

#include <Common/Assert.h>

#include <ECS/Entity.h>
#include <ECS/Components/ComponentPool.h>
#include <ECS/Systems/CommonSystem.h>

#include <array>
#include <unordered_map>
#include <vector>

// =================================================================================================
// Entity Component System Interface
// =================================================================================================
namespace CYD
{
class BaseComponentPool;
class BaseSharedComponent;

namespace ECS
{
namespace detail
{
using Entities         = std::unordered_map<EntityHandle, Entity>;
using Components       = std::array<BaseComponentPool*, size_t( ComponentType::COUNT )>;
using SharedComponents = std::array<BaseSharedComponent*, size_t( SharedComponentType::COUNT )>;
using Systems          = std::vector<BaseSystem*>;

// All entities currently managed by the manager (all entities in the world)
inline Entities entities;

// Pools of all components. Index is component type.
inline Components components;
inline SharedComponents sharedComponents;

// All currently running data transformation systems
inline Systems systems;
}

// Initialization and update
// ================================================================================================
bool Initialize();
void Uninitialize();

void Tick( double deltaS );

// Entity management
// ================================================================================================
EntityHandle CreateEntity();
void RemoveEntity( EntityHandle handle );

// Shared component accessor
// ================================================================================================
template <
    class Component,
    typename = std::enable_if_t<std::is_base_of_v<BaseSharedComponent, Component>>>
Component& GetSharedComponent()
{
   return *static_cast<Component*>( detail::sharedComponents[size_t( Component::TYPE )] );
}

// Adding system
// ================================================================================================
template <
    class System,
    typename... Args,
    typename = std::enable_if_t<std::is_base_of_v<BaseSystem, System>>>
void AddSystem( Args&&... args )
{
   System* system = new System( std::forward<Args>( args )... );
   system->init();
   detail::systems.emplace_back( system );
}

// Component assignment
// ================================================================================================
template <class Component, typename... Args>
void Assign( EntityHandle handle, Args&&... args )
{
   auto it = detail::entities.find( handle );
   if( it == detail::entities.end() )
   {
      CYDASSERT( !"ECS: Could not find entity" );
      return;
   }

   Component* pComponent = nullptr;

   // Fetching component from adequate pool
   if constexpr( std::is_base_of_v<BaseComponent, Component> )
   {
      // Component is a normal component
      ComponentPool<Component>*& pPool =
          (ComponentPool<Component>*&)detail::components[size_t( Component::TYPE )];
      if( !pPool )
      {
         // Pool has never been created, create it
         pPool = new ComponentPool<Component>();
      }

      pComponent = pPool->acquireComponent( std::forward<Args>( args )... );
   }
   else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
   {
      // Component is a shared component
      pComponent = static_cast<Component*>( detail::sharedComponents[size_t( Component::TYPE )] );
   }
   else
   {
      static_assert( !"ECS: Assigning an invalid component" );
   }

   it->second.addComponent<Component>( pComponent );

   // Notify systems that an entity was assigned a component
   for( auto& system : detail::systems )
   {
      system->onEntityAssigned( it->second );
   }
}

// Component unassignment
// ================================================================================================
template <class Component>
void Unassign( EntityHandle handle )
{
   auto it = detail::entities.find( handle );
   if( it == detail::entities.end() )
   {
      CYDASSERT( !"ECS: Could not find entity" );
      return;
   }

   if constexpr( std::is_base_of_v<BaseComponent, Component> )
   {
      // Component is a normal component
      ComponentPool<Component>*& pPool =
          (ComponentPool<Component>*&)detail::components[size_t( Component::TYPE )];

      // Deallocate it from the pool
      pPool->releaseComponent( it->second.getComponent() );
   }
   else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
   {
      // Nothing to deallocate
   }
   else
   {
      static_assert( !"ECS: Unassigning an invalid component" );
   }

   it->second.removeComponent<Component>();

   for( auto& system : detail::systems )
   {
      system->onEntityUnassigned( it->second );
   }
}
}  
}
