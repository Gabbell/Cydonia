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

   std::vector<EntityEntry> m_entities;

  public:
   NON_COPIABLE( CommonSystem );
   virtual ~CommonSystem() = default;

   // If the system is not watching any entity, no need to tick
   bool hasToTick() const noexcept override { return !m_entities.empty(); }

   // Override this function in your system to tell it how the entities should be inserted/sorted.
   // By default, the entities are in the same order they were assigned to the system
   static bool upperBound( const EntityEntry&, const EntityEntry& ) { return true; }

   void onEntityAssigned( const Entity& entity ) override final
   {
      EntityEntry entry;
      entry.handle = entity.getHandle();

      // Make sure that if the entity previously matched, we are not doubling components
      const auto it = std::find_if(
          m_entities.cbegin(), m_entities.cend(), [entry]( const EntityEntry& desiredEntry ) {
             return entry.handle == desiredEntry.handle;
          } );

      if( it == m_entities.cend() )
      {
         uint32_t matches = 0;

         _processEntityArchetype<0, Components...>( entity, matches, entry.arch );

         // Check if we have a match!
         if( matches == sizeof...( Components ) )
         {
            // Insert with optional upperBound predicate
            m_entities.insert(
                std::upper_bound( m_entities.cbegin(), m_entities.cend(), entry, upperBound ),
                std::move( entry ) );
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
              [&entity]( const EntityEntry& entry ) {
                 return entity.getHandle() == entry.handle;
              } ),
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
