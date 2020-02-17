#pragma once

#include <Common/Include.h>
#include <Common/Assert.h>

#include <ECS/Components/ComponentTypes.h>
#include <ECS/SharedComponents/SharedComponentType.h>

#include <limits>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class BaseComponent;
class BaseSharedComponent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
using EntityHandle = size_t;

class Entity final
{
  public:
   Entity() = default;
   Entity( EntityHandle handle ) : m_handle( handle ) {}
   MOVABLE( Entity )
   ~Entity() = default;

   EntityHandle getHandle() const noexcept { return m_handle; }

   // Adding component pointer to entity
   // ==============================================================================================
   template <class Component>
   void addComponent( Component* pComponent )
   {
      CYDASSERT( pComponent && "Entity: Tried to assign a null component" );

      if constexpr( std::is_base_of_v<BaseComponent, Component> )
      {
         auto it = m_components.find( Component::TYPE );
         if( it != m_components.end() )
         {
            // Component has already been assigned to this entity
            CYDASSERT( "!Entity: Cannot overwrite components" );
            return;
         }

         m_components[Component::TYPE] = pComponent;
      }
      else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
      {
         auto it = m_sharedComponents.find( Component::TYPE );
         if( it != m_sharedComponents.end() )
         {
            // Component has already been assigned to this entity
            CYDASSERT( "!Entity: Cannot overwrite components" );
            return;
         }

         m_sharedComponents[Component::TYPE] = pComponent;
      }
      else
      {
         static_assert( !"Entity: Trying to add an invalid component to an entity" );
      }
   }

   // Remove component pointer from entity
   // ==============================================================================================
   template <class Component>
   void removeComponent()
   {
      if constexpr( std::is_base_of_v<BaseComponent, Component> )
      {
         m_components.erase( Component::TYPE );
      }
      else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
      {
         m_sharedComponents.erase( Component::TYPE );
      }
      else
      {
         static_assert( !"Entity: Trying to remove an invalid component" );
      }
   }

   // Accessors
   // ==============================================================================================
   template <class Component>
   Component* getComponent() const
   {
      if constexpr( std::is_base_of_v<BaseComponent, Component> )
      {
         auto it = m_components.find( Component::TYPE );
         if( it != m_components.end() )
         {
            return it->second;
         }
         return nullptr;
      }
      else if constexpr( std::is_base_of_v<BaseSharedComponent, Component> )
      {
         auto it = m_sharedComponents.find( SharedComponent::TYPE );
         if( it != m_sharedComponents.end() )
         {
            return it->second;
         }
         return nullptr;
      }
      else
      {
         static_assert( !"Entity: Trying to get an invalid component" );
      }

      return nullptr;
   }

   using ComponentsMap       = std::unordered_map<ComponentType, BaseComponent*>;
   using SharedComponentsMap = std::unordered_map<SharedComponentType, BaseSharedComponent*>;

   const ComponentsMap& getComponents() const { return m_components; }
   const SharedComponentsMap& getSharedComponents() const { return m_sharedComponents; }

   static constexpr EntityHandle INVALID_HANDLE = std::numeric_limits<size_t>::max();

  private:
   // This entity's handle
   EntityHandle m_handle = INVALID_HANDLE;

   // All components associated (that were added) to this entity.
   ComponentsMap m_components;
   SharedComponentsMap m_sharedComponents;
};
}
