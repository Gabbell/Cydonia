#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/Handle.h>

#include <cstdint>

namespace cyd
{
class HandleManager
{
  public:
   HandleManager();
   NON_COPIABLE( HandleManager );
   ~HandleManager() = default;

   void reset();
   Handle add( void* data, HandleType handleType );
   void update( Handle handle, void* newData );
   void remove( Handle handle );

   void* get( Handle handle ) const;
   bool get( Handle handle, void*& out ) const;

   int getCount() const { return m_activeEntryCount; }

   template <typename T>
   bool getAs( Handle handle, T& out ) const
   {
      void* outAsVoid;
      const bool rv = get( handle, outAsVoid );

      out = union_cast<T>( outAsVoid );

      return rv;
   };

  private:
   static constexpr uint32_t MAX_ENTRIES = 512;

   struct HandleEntry
   {
      HandleEntry();
      explicit HandleEntry( uint32_t nextFreeIndex );

      uint32_t nextFreeIndex : 12;
      uint32_t counter : 15;
      uint32_t active : 1;
      uint32_t endOfList : 1;
      void* entry;
   };

   HandleEntry m_entries[MAX_ENTRIES];

   int m_activeEntryCount;
   uint32_t m_firstFreeEntry;
};
}
