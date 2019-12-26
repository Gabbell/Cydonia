#pragma once

#include <Common/Include.h>

#include <ECS/Components/ComponentTypes.h>

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
   explicit Entity( EntityHandle handle ) : m_handle( handle ) {}
   MOVABLE( Entity );
   ~Entity() = default;

   void addComponent( ComponentType type, BaseComponent* pComponent );
   void removeComponent( ComponentType type );

   // Retrieve the entity's handle
   EntityHandle getHandle() const { return m_handle; }

   const ComponentsMap& getComponentsMap() const { return m_components; }

   template <class Component>
   Component* getComponent() const;

   static constexpr EntityHandle INVALID_HANDLE = 0;

  private:
   // All components associated (that were added) to this entity.
   ComponentsMap m_components;

   EntityHandle m_handle = INVALID_HANDLE;
};

template <class Component>
Component* Entity::getComponent() const
{
   auto it = m_components.find( Component::TYPE );
   if( it != m_components.end() )
   {
      return it->second;
   }
   return nullptr;
}
}
