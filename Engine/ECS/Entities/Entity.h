#pragma once

#include <Common/Include.h>
#include <Common/Assert.h>

#include <ECS/Components/ComponentTypes.h>

#include <limits>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class BaseComponent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
using EntityHandle = size_t;

class Entity final
{
   using ComponentsMap = std::unordered_map<ComponentType, BaseComponent*>;

  public:
   Entity() = default;
   MOVABLE( Entity );
   ~Entity() = default;

   template <class Component>
   void addComponent( Component* pComponent )
   {
      static_assert(
          (std::is_base_of_v<BaseComponent, Component>),
          "Attempting to add an invalid component to an entity" );

      auto it = m_components.find( Component::TYPE );

      if( it != m_components.end() )
      {
         // Component has already been assigned to this entity
         CYDASSERT_AND_RETURN( "Entity: Cannot overwrite components" );
      }

      m_components[Component::TYPE] = pComponent;
   }

   template <class Component>
   void removeComponent()
   {
      static_assert(
          (std::is_base_of_v<BaseComponent, Component>),
          "Attempting to remove an invalid component from an entity" );

      m_components.erase( Component::TYPE );
   }

   const ComponentsMap& getComponentsMap() const { return m_components; }

   template <class Component>
   Component* getComponent() const
   {
      static_assert(
          (std::is_base_of_v<BaseComponent, Component>),
          "Attempting to get an invalid component from an entity" );

      auto it = m_components.find( Component::TYPE );
      if( it != m_components.end() )
      {
         return it->second;
      }
      return nullptr;
   }

   static constexpr EntityHandle INVALID_HANDLE = std::numeric_limits<size_t>::max();

  private:
   // All components associated (that were added) to this entity.
   ComponentsMap m_components;
};
}
