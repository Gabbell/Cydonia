#pragma once

#include <Common/Include.h>

#include <ECS/Entity.h>

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
   NON_COPIABLE( BaseSystem )
   virtual ~BaseSystem() = default;

   virtual bool init()   = 0;
   virtual void uninit() = 0;

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

  public:
   NON_COPIABLE( CommonSystem )
   virtual ~CommonSystem() = default;

   // If the system is not watching any entity, no need to tick
   bool hasToTick() const noexcept override { return !m_components.empty(); }

   void onEntityAssigned( const Entity& entity ) override final
   {
      EntityHandle handle = entity.getHandle();

      // Make sure that if the entity previously matched, we are not doubling components
      if( m_components.find( handle ) == m_components.end() )
      {
         Archetype arch;
         uint32_t matches = 0;

         _processEntityArchetype<0, Components...>( entity, matches, arch );

         // Check if we have a match!
         if( matches == sizeof...( Components ) )
         {
            m_components[handle] = std::move( arch );
         }
      }
   }

   void onEntityUnassigned( const Entity& entity ) override final
   {
      m_components.erase( entity.getHandle() );
   }

  protected:
   CommonSystem() = default;

   // The archetype only includes the normal components as they are the only ones worth tracking
   // since they are per entity. We therefore filter out anything else from the parameter pack
   using Archetype = decltype( std::tuple_cat(
       std::declval<std::conditional_t<
           std::is_base_of_v<BaseComponent, Components>,
           std::tuple<std::add_pointer_t<Components>>,
           std::tuple<>>>()... ) );

   std::unordered_map<EntityHandle, Archetype> m_components;

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
