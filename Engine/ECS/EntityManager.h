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
}

namespace CYD
{
class EntityManager final
{
  public:
   EntityManager();
   NON_COPIABLE( EntityManager );
   ~EntityManager();

   void tick( double deltaS );

   // Entity management
   // ================================================================================================
   EntityHandle createEntity();
   const Entity* getEntity( EntityHandle handle ) const;
   void removeEntity( EntityHandle handle );

   // Shared component accessor
   // ================================================================================================
   template <
       class SharedComponent,
       typename = std::enable_if_t<std::is_base_of_v<BaseSharedComponent, SharedComponent>>>
   SharedComponent& getSharedComponent()
   {
      return *static_cast<SharedComponent*>( m_sharedComponents[(size_t)SharedComponent::TYPE] );
   }

   // Adding system
   // ================================================================================================
   template <
       class System,
       typename... Args,
       typename = std::enable_if_t<std::is_base_of_v<BaseSystem, System>>>
   void addSystem( Args&&... args )
   {
      System* newSystem = new System( std::forward<Args>(args)... );
      newSystem->assignEntityManager( this );

      m_systems.push_back( newSystem );
   }

   // Component assignment
   // ================================================================================================
   template <class Component, typename... Args>
   void assign( EntityHandle handle, Args&&... args )
   {
      auto it = m_entities.find( handle );
      if( it == m_entities.end() )
      {
         CYDASSERT( !"ECS: Could not find entity" );
         return;
      }

      Component* pComponent = nullptr;

      // Fetching component from adequate pool
      if constexpr( std::is_base_of_v<BaseComponent, Component> )
      {
         // Index of the component pool
         constexpr size_t componentPoolIdx = static_cast<size_t>( Component::TYPE );

         static_assert(
             componentPoolIdx != static_cast<size_t>( ComponentType::UNKNOWN ) &&
             "ECS: Component pool index is unknown" );

         // Component is a normal component
         ComponentPool<Component>*& pPool =
             (ComponentPool<Component>*&)m_components[componentPoolIdx];
         if( !pPool )
         {
            // Pool has never been created, create it
            pPool = new ComponentPool<Component>();
         }

         pComponent = pPool->acquireComponent( std::forward<Args>( args )... );
      }
      else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
      {
         // Index of the component pool
         constexpr size_t componentPoolIdx = static_cast<size_t>( Component::TYPE );

         // Component is a shared component
         pComponent = static_cast<Component*>( m_sharedComponents[componentPoolIdx] );
      }
      else
      {
         static_assert( !"ECS: Assigning an invalid component" );
      }

      it->second.addComponent<Component>( pComponent );

      // Notify systems that an entity was assigned a component
      for( auto& system : m_systems )
      {
         system->onEntityAssigned( it->second );
      }
   }

   // Component unassignment
   // ================================================================================================
   template <class Component>
   void unassign( EntityHandle handle )
   {
      auto it = m_entities.find( handle );
      if( it == m_entities.end() )
      {
         CYDASSERT( !"ECS: Could not find entity" );
         return;
      }

      if constexpr( std::is_base_of_v<BaseComponent, Component> )
      {
         // Component is a normal component
         ComponentPool<Component>*& pPool =
             (ComponentPool<Component>*&)m_components[(size_t)Component::TYPE];

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

      for( auto& system : m_systems )
      {
         system->onEntityUnassigned( it->second );
      }
   }

  private:
   using Entities         = std::unordered_map<EntityHandle, Entity>;
   using Components       = std::array<BaseComponentPool*, (size_t)ComponentType::COUNT>;
   using SharedComponents = std::array<BaseSharedComponent*, (size_t)SharedComponentType::COUNT>;
   using Systems          = std::vector<BaseSystem*>;

   // All entities currently managed by the manager (all entities in the world)
   Entities m_entities;

   // Pools of all components. Index is component type.
   Components m_components;
   SharedComponents m_sharedComponents;

   // All currently running data transformation systems
   Systems m_systems;
};
}