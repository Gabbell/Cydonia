#pragma once

#include <Common/Include.h>

#include <ECS/Entity.h>

#include <algorithm>
#include <utility>

// ================================================================================================
// Forwards
// ================================================================================================
namespace CYD
{
class Entity;
class EntityManager;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class BaseSystem
{
  public:
   NON_COPIABLE( BaseSystem );
   virtual ~BaseSystem() = default;

   virtual void sort() = 0;

   virtual bool hasToTick() const noexcept = 0;
   virtual void tick( double deltaS )      = 0;

   virtual void onEntityAssigned( const Entity& entity )   = 0;
   virtual void onEntityUnassigned( const Entity& entity ) = 0;

  protected:
   BaseSystem() = default;
};

template <class... Components>
class CommonSystem : public BaseSystem
{
   static_assert(
       ( (std::is_base_of_v<BaseComponent, Components> ||
          std::is_base_of_v<BaseSharedComponent, Components>)&&... ) );

  protected:
   CommonSystem() = default;

   // The archetype only includes the normal components as they are the only ones worth tracking
   // since they are per entity. We therefore filter out anything else from the parameter pack
   using Archetype = decltype( std::tuple_cat( std::declval<std::conditional_t<
                                                   std::is_base_of_v<BaseComponent, Components>,
                                                   std::tuple<std::add_pointer_t<Components>>,
                                                   std::tuple<>>>()... ) );

   // This is how systems keep track of its entities
   struct EntityEntry
   {
      EntityHandle handle;
      Archetype arch;
   };

   EntityManager* m_ecs = nullptr;
   std::vector<EntityEntry> m_entities;

   bool m_keepSortedAtAllTimes : 1 = false;

   // This function is used when inserting new entities into a system. Only use this if you need
   // entities to be sorted at all times. Otherwise, override the sort function
   // By default, the entities are in the same order they were assigned to the system. Return true
   // if the first argument is "less" (ordered before) than the second.
   virtual bool _compareEntities( const EntityEntry&, const EntityEntry& ) { return true; }

  public:
   NON_COPIABLE( CommonSystem );
   virtual ~CommonSystem() = default;

   void assignEntityManager( EntityManager* ecs ) { m_ecs = ecs; }

   // If the system is not watching any entity, no need to tick
   bool hasToTick() const noexcept override { return !m_entities.empty(); }

   // Override to sort entities in any way desirable
   void sort() override { return; }

   void onEntityAssigned( const Entity& entity ) override final
   {
      EntityEntry entry;
      entry.handle = entity.getHandle();

      // Make sure that if the entity previously matched, we are not doubling components
      const auto it = std::find_if(
          m_entities.cbegin(),
          m_entities.cend(),
          [entry]( const EntityEntry& desiredEntry )
          { return entry.handle == desiredEntry.handle; } );

      if( it == m_entities.cend() )
      {
         uint32_t matches = 0;

         _processEntityArchetype<0, Components...>( entity, matches, entry.arch );

         // Check if we have a match!
         if( matches > 0 && matches == sizeof...( Components ) )
         {
            // Insert with the optional upperbound predicate
            if( m_keepSortedAtAllTimes )
            {
               m_entities.insert(
                   std::upper_bound(
                       m_entities.cbegin(),
                       m_entities.cend(),
                       entry,
                       [this]( const EntityEntry& first, const EntityEntry& second )
                       { return _compareEntities( first, second ); } ),
                   std::move( entry ) );
            }

            m_entities.push_back( std::move( entry ) );
         }
      }
   }

   void onEntityUnassigned( const Entity& entity ) override final
   {
      // Could also just erase_if since we know there should be only one instance of an entity
      m_entities.erase(
          std::remove_if(
              m_entities.begin(),
              m_entities.end(),
              [&entity]( const EntityEntry& entry )
              { return entity.getHandle() == entry.handle; } ),
          m_entities.end() );
   }

  private:
   template <size_t INDEX, class Component, class... Args>
   void _processEntityArchetype( const Entity& entity, uint32_t& matches, Archetype& archToFill )
   {
      // This function will look through the required components of this particular system and
      // see if there is a match with the entity. Only the intersection of both the entity's
      // components and the system's are registered into the archetype
      if constexpr( std::is_base_of_v<BaseComponent, Component> )
      {
         for( const auto& compPair : entity.getComponents() )
         {
            if( Component::TYPE == compPair.first )
            {
               matches++;
               std::get<INDEX>( archToFill ) = static_cast<Component*>( compPair.second );
            }
         }
         _processEntityArchetype<INDEX + 1, Args...>( entity, matches, archToFill );
      }
      else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
      {
         for( const auto& compPair : entity.getSharedComponents() )
         {
            if( Component::TYPE == compPair.first )
            {
               // Shared components are checked for match only. We do not add them to the archetype
               matches++;
            }
         }
         _processEntityArchetype<INDEX, Args...>( entity, matches, archToFill );
      }
   }

   template <size_t INDEX>
   bool _processEntityArchetype( const Entity&, uint32_t&, Archetype& )
   {
      return false;
   }
};
}
