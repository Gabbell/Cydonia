#pragma once

#include <Common/Assert.h>
#include <Common/Include.h>

#include <array>

namespace EMP
{
template <class OBJECT_TYPE, size_t MAX_SIZE = 1024 * 128 /*128kB by default*/>
class ObjectPool
{
  public:
   ObjectPool() = default;
   NON_COPIABLE( ObjectPool );
   ~ObjectPool() = default;

   static constexpr size_t INVALID_POOL_IDX = std::numeric_limits<size_t>::max();

   using Index = size_t;

   size_t getCount() const { return m_count; }

   template <typename... Args>
   Index insertObject( Args&&... args )
   {
      for( uint32_t i = 0; i < m_slots.size(); ++i )
      {
         if( !m_slots[i] )
         {
            // A free slot was found
            m_objects[i] = OBJECT_TYPE( std::forward<Args>( args )... );
            m_slots[i]   = true;
            m_count++;
            return i;
         }
      }

      CYD_ASSERT( !"ObjectPool: Ran out of slots" );
      return INVALID_POOL_IDX;
   }

   OBJECT_TYPE* operator[]( Index idx )
   {
      if( m_slots[idx] )
      {
         return &m_objects[idx];
      }

      return nullptr;
   }

   const OBJECT_TYPE* operator[]( Index idx ) const
   {
      if( m_slots[idx] )
      {
         return &m_objects[idx];
      }

      return nullptr;
   }

   void releaseObject( Index idx )
   {
      assert( idx != INVALID_POOL_IDX && "ObjectPool: Trying to release an invalid component" );

      m_slots[idx]   = false;  // Freeing slot
      m_objects[idx] = {};
      m_count--;
   }

  private:
   static constexpr size_t MAX_ELEMENT_COUNT = MAX_SIZE / sizeof( OBJECT_TYPE );
   static_assert( MAX_ELEMENT_COUNT > 0 );

   std::array<bool, MAX_ELEMENT_COUNT> m_slots          = {};
   std::array<OBJECT_TYPE, MAX_ELEMENT_COUNT> m_objects = {};

   size_t m_count = 0;
};
}