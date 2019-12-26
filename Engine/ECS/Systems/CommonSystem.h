#pragma once

#include <Common/Include.h>

#include <ECS/Systems/BaseSystem.h>

#include <ECS/Entities/Entity.h>

#include <ECS/Components/ComponentTypes.h>

#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class EntityManager;
class BaseComponent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
template <class... ComponentTypes>
class CommonSystem : public BaseSystem
{
  public:
   explicit CommonSystem( const EntityManager& entityManager ) : m_entityManager( entityManager ) {}
   NON_COPIABLE( CommonSystem );
   virtual ~CommonSystem() = default;

   void onEntityCreate( const Entity& entity ) override final;
   void onEntityDestroy( const Entity& entity ) override final;

  protected:
   using CompTuple = std::tuple<std::add_pointer_t<ComponentTypes>...>;

   // All the components of the entities this system manages
   std::vector<CompTuple> m_components;

  private:
   template <size_t INDEX, class Component, class... Args>
   bool
   processEntityComponent( ComponentType type, BaseComponent* pComponent, CompTuple& tupleToFill );

   template <size_t INDEX>
   bool
   processEntityComponent( ComponentType type, BaseComponent* pComponent, CompTuple& tupleToFill );

   const EntityManager& m_entityManager;
};

template <class... ComponentTypes>
template <size_t INDEX, class Component, class... Args>
bool CommonSystem<ComponentTypes...>::processEntityComponent(
    ComponentType type,
    BaseComponent* pComponent,
    CompTuple& tupleToFill )
{
   if( Component::TYPE == type )
   {
      std::get<INDEX>( tupleToFill ) = static_cast<Component*>( pComponent );
      return true;
   }

   return processEntityComponent<INDEX + 1, Args...>( type, pComponent, tupleToFill );
}

template <class... ComponentTypes>
template <size_t INDEX>
bool CommonSystem<ComponentTypes...>::processEntityComponent(
    ComponentType,
    BaseComponent*,
    CompTuple& )
{
   return false;
}

template <class... ComponentTypes>
void CommonSystem<ComponentTypes...>::onEntityCreate( const Entity& entity )
{
   CompTuple compTuple;
   size_t matchingComps = 0;
   for( const auto& compPair : entity.getComponentsMap() )
   {
      if( processEntityComponent<0, ComponentTypes...>(
              compPair.first, compPair.second, compTuple ) )
      {
         matchingComps++;
         if( matchingComps == sizeof...( ComponentTypes ) )
         {
            m_components.push_back( std::move( compTuple ) );
            break;
         }
      }
   }
}

template <class... ComponentTypes>
void CommonSystem<ComponentTypes...>::onEntityDestroy( const Entity& entity )
{
   //
}
}
