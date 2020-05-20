#include <Handles/HandleManager.h>

#include <Common/Assert.h>

#include <cstddef>

namespace CYD
{
HandleManager::HandleEntry::HandleEntry()
    : _nextFreeIndex( 0 ), _counter( 1 ), _active( 0 ), _endOfList( 0 ), _entry( nullptr )
{
}

HandleManager::HandleEntry::HandleEntry( uint32_t nextFreeIndex )
    : _nextFreeIndex( nextFreeIndex ), _counter( 1 ), _active( 0 ), _endOfList( 0 ), _entry( NULL )
{
}

HandleManager::HandleManager() { reset(); }

void HandleManager::reset()
{
   _activeEntryCount = 0;
   _firstFreeEntry   = 0;

   for( int i = 0; i < MAX_ENTRIES - 1; ++i ) _entries[i] = HandleEntry( i + 1 );
   _entries[MAX_ENTRIES - 1]            = HandleEntry();
   _entries[MAX_ENTRIES - 1]._endOfList = true;
}

Handle HandleManager::add( void* p, HandleType handleType )
{
   uint32_t type = static_cast<uint32_t>( handleType );

   CYDASSERT( _activeEntryCount < ( MAX_ENTRIES - 1 ) );

   CYDASSERT( type >= 0 && type <= 31 );

   const int newIndex = _firstFreeEntry;
   CYDASSERT( newIndex < MAX_ENTRIES );
   CYDASSERT( _entries[newIndex]._active == false );
   CYDASSERT( !_entries[newIndex]._endOfList );

   _firstFreeEntry                   = _entries[newIndex]._nextFreeIndex;
   _entries[newIndex]._nextFreeIndex = 0;
   _entries[newIndex]._counter       = _entries[newIndex]._counter + 1;
   if( _entries[newIndex]._counter == 0 )
   {
      _entries[newIndex]._counter = 1;
   }
   _entries[newIndex]._active = true;
   _entries[newIndex]._entry  = p;

   ++_activeEntryCount;

   return Handle( newIndex, _entries[newIndex]._counter, handleType );
}

void HandleManager::update( Handle handle, void* newData )
{
   const int index = handle._index;
   CYDASSERT( _entries[index]._counter == handle._counter );
   CYDASSERT( _entries[index]._active == true );

   _entries[index]._entry = newData;
}

void HandleManager::remove( const Handle handle )
{
   const uint32_t index = handle._index;
   CYDASSERT( _entries[index]._counter == handle._counter );
   CYDASSERT( _entries[index]._active == true );

   _entries[index]._nextFreeIndex = _firstFreeEntry;
   _entries[index]._active        = 0;
   _firstFreeEntry                = index;

   --_activeEntryCount;
}

void* HandleManager::get( Handle handle ) const
{
   void* p = nullptr;
   if( !get( handle, p ) ) return nullptr;
   return p;
}

bool HandleManager::get( const Handle handle, void*& out ) const
{
   const int index = handle._index;
   if( _entries[index]._counter != handle._counter || _entries[index]._active == false )
      return false;

   out = _entries[index]._entry;
   return true;
}
}
