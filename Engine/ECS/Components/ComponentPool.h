#pragma once

#include <Common/Include.h>

#include <array>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class BaseComponent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class BaseComponentPool
{
  public:
   NON_COPIABLE( BaseComponentPool );
   virtual ~BaseComponentPool() = default;

   virtual void releaseComponent( int32_t poolIdx ) = 0;

  protected:
   BaseComponentPool() = default;
};

template <class Component, typename = std::enable_if_t<std::is_base_of_v<BaseComponent, Component>>>
class ComponentPool final : public BaseComponentPool
{
  public:
   ComponentPool() = default;

   NON_COPIABLE( ComponentPool );
   virtual ~ComponentPool() = default;

   static constexpr size_t INVALID_POOL_IDX = std::numeric_limits<size_t>::max();

   // static_assert(
   //     std::is_trivially_move_assignable_v<Component> &&
   //     "ComponentPool: Component is not move assignable, this can hurt performances" );

   Component* getComponent( size_t index ) const { return m_components[index]; }

   template <typename... Args>
   Component* acquireComponent( Args&&... args )
   {
      for( uint32_t i = 0; i < m_slots.size(); ++i )
      {
         if( !m_slots[i] )
         {
            // A free slot was found
            m_components[i] = Component( std::forward<Args>( args )... );

            m_components[i].setPoolIndex( i );

            m_slots[i] = true;
            return &m_components[i];
         }
      }

      CYDASSERT( !"ComponentPool: Ran out of slots" );
      return nullptr;
   }

   void releaseComponent( int32_t poolIdx ) override
   {
      CYDASSERT( poolIdx >= 0 && "ComponentPool: Trying to release an invalid component" );

      m_slots[poolIdx] = false;  // Freeing slot
   }

  private:
   static constexpr size_t MAX_POOL_SIZE        = 1024 * 64;  // 64kB
   static constexpr size_t COMPONENT_ARRAY_SIZE = MAX_POOL_SIZE / sizeof( Component );

   std::array<bool, COMPONENT_ARRAY_SIZE> m_slots           = {};
   std::array<Component, COMPONENT_ARRAY_SIZE> m_components = {};
};
}
