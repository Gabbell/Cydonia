#pragma once

#include <Common/Include.h>

#include <ECS/Entities/Entity.h>

#include <tuple>
#include <unordered_map>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class BaseSystem;
class BaseComponent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class EntityManager final
{
  public:
   EntityManager() = default;
   NON_COPIABLE( EntityManager )
   ~EntityManager();

   // Initialize all systems
   bool init();

   void tick( double deltaMs );

   EntityHandle addEntity();

   void removeEntity( EntityHandle handle );

  private:
   using Entities   = std::unordered_map<EntityHandle, Entity>;
   using Components = std::vector<std::vector<BaseComponent*>>;
   using Systems    = std::vector<BaseSystem*>;

   // All entities currently managed by the manager (all entities in the world)
   Entities m_entities;

   // Table of all components (first is component type, second is pool index)
   Components m_components;

   // All currently running data transformation systems
   Systems m_systems;
};
}
