#pragma once

#include <Graphics/Handles/ResourceHandle.h>

#include <cstdint>

namespace CYD
{
class HandleManager
{
  public:
   HandleManager();
   HandleManager( const HandleManager& ) = delete;
   HandleManager& operator=( const HandleManager& ) = delete;
   ~HandleManager()                                 = default;

   void reset();
   Handle add( void* data, HandleType handleType );
   void update( Handle handle, void* newData );
   void remove( Handle handle );

   void* get( Handle handle ) const;
   bool get( Handle handle, void*& out ) const;

   int getCount() const { return _activeEntryCount; }

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

      uint32_t _nextFreeIndex : 12;
      uint32_t _counter : 15;
      uint32_t _active : 1;
      uint32_t _endOfList : 1;
      void* _entry;
   };

   HandleEntry _entries[MAX_ENTRIES];

   int _activeEntryCount;
   uint32_t _firstFreeEntry;
};
}
