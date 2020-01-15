#pragma once

#include <Common/Include.h>

#include <ECS/Components/BaseComponent.h>

#include <array>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class BaseComponentPool
{
  protected:
   BaseComponentPool() = default;
};

template <class Component>
class ComponentPool final : public BaseComponentPool
{
  public:
   ComponentPool()
   {
      static_assert(
          (std::is_base_of_v<BaseComponent, Component>),
          "Attempting to create an archetype pool with an invalid component" );
   }
   NON_COPIABLE( ComponentPool );
   ~ComponentPool() = default;

   static constexpr size_t INVALID_POOL_IDX = std::numeric_limits<size_t>::max();

   Component* getComponent( size_t index ) const { return m_components[index]; }

   template <typename... Args>
   Component* acquireComponent( Args&&... args )
   {
      for( uint32_t i = 0; i < m_slots.size(); ++i )
      {
         if( !m_slots[i] )
         {
            // A free slot was found
            m_components[i] = Component( args... );
            m_components[i].init();

            m_slots[i] = true;
            return &m_components[i];
         }
      }

      CYDASSERT( !"ComponentPool: Ran out of slots" );
      return nullptr;
   }

   void releaseComponent()
   {
      //
   }

  private:
   static constexpr size_t MAX_POOL_SIZE        = 1024 * 8;  // 8kB
   static constexpr size_t COMPONENT_ARRAY_SIZE = MAX_POOL_SIZE / sizeof( Component );

   std::array<bool, COMPONENT_ARRAY_SIZE> m_slots           = {};
   std::array<Component, COMPONENT_ARRAY_SIZE> m_components = {};
};
}