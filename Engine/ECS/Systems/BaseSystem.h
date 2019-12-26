#pragma once

#include <Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Entity;
class EntityManager;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class BaseSystem
{
  public:
   BaseSystem() = default;
   NON_COPIABLE( BaseSystem );
   virtual ~BaseSystem() = default;

   virtual bool init()                 = 0;
   virtual void tick( double deltaMs ) = 0;

   virtual void onEntityCreate( const Entity& entity )  = 0;
   virtual void onEntityDestroy( const Entity& entity ) = 0;
};
}
