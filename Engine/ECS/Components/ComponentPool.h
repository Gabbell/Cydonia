#pragma once

#include <Common/Include.h>

#include <vector>

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
template <class Component>
class ComponentPool final
{
  public:
   ComponentPool() = default;
   NON_COPIABLE( ComponentPool );
   ~ComponentPool() = default;

   bool init( size_t poolSize )
   {
      static_assert(
          std::is_base_of<BaseComponent, Component>::value,
          "Can't create a component pool with a class that does not inherit BaseComponent" );

      m_componentPool.resize( poolSize );
      return true;
   }

  private:
   std::vector<Component> m_componentPool;

   // Number of components in the pool
   size_t m_poolSize = 0;
};
}
