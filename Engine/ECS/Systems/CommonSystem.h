#pragma once

#include <Common/Include.h>

#include <ECS/Entities/Entity.h>
#include <ECS/Components/ComponentTypes.h>

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Entity;
class BaseComponent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class BaseSystem
{
  public:
   NON_COPIABLE( BaseSystem );
   virtual ~BaseSystem() = default;

   virtual bool init()                 = 0;
   virtual void tick( double deltaMs ) = 0;

   virtual void onEntityAssigned( const Entity& entity )   = 0;
   virtual void onEntityUnassigned( const Entity& entity ) = 0;

  protected:
   BaseSystem() = default;
};

template <class... Components>
class CommonSystem : public BaseSystem
{
  public:
   NON_COPIABLE( CommonSystem );
   virtual ~CommonSystem() = default;

   void onEntityAssigned( const Entity& entity ) override final
   {
      Archetype arch;
      size_t matchingArchs = 0;
      for( const auto& compPair : entity.getComponentsMap() )
      {
         if( processEntityArchetype<0, Components...>( compPair.first, compPair.second, arch ) )
         {
            matchingArchs++;
            if( matchingArchs == sizeof...( Components ) )
            {
               m_archetypes.push_back( std::move( arch ) );
               break;
            }
         }
      }
   }

   void onEntityUnassigned( const Entity& entity ) override final
   {
      //
   }

  protected:
   CommonSystem() = default;

   using Archetype = std::tuple<std::add_pointer_t<Components>...>;
   std::vector<Archetype> m_archetypes;

  private:
   template <size_t INDEX, class Component, class... Args>
   bool
   processEntityArchetype( ComponentType type, BaseComponent* pComponent, Archetype& archToFill )
   {
      if( Component::TYPE == type )
      {
         std::get<INDEX>( archToFill ) = static_cast<Component*>( pComponent );
         return true;
      }

      return processEntityArchetype<INDEX + 1, Args...>( type, pComponent, archToFill );
   }

   template <size_t INDEX>
   bool processEntityArchetype( ComponentType, BaseComponent*, Archetype& )
   {
      return false;
   }
};
}
