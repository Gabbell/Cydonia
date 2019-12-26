#include <Graphics/Handles/HandleManager.h>

#include <Common/Assert.h>

#include <cstddef>

namespace cyd
{
HandleManager::HandleEntry::HandleEntry()
    : nextFreeIndex( 0 ), counter( 1 ), active( 0 ), endOfList( 0 ), entry( nullptr )
{
}

HandleManager::HandleEntry::HandleEntry( uint32_t nextFreeIndex )
    : nextFreeIndex( nextFreeIndex ), counter( 1 ), active( 0 ), endOfList( 0 ), entry( NULL )
{
}

HandleManager::HandleManager() { reset(); }

void HandleManager::reset()
{
   m_activeEntryCount = 0;
   m_firstFreeEntry   = 0;

   for( int i = 0; i < MAX_ENTRIES - 1; ++i ) m_entries[i] = HandleEntry( i + 1 );
   m_entries[MAX_ENTRIES - 1]             = HandleEntry();
   m_entries[MAX_ENTRIES - 1].endOfList = true;
}

Handle HandleManager::add( void* p, HandleType handleType )
{
   uint32_t type = static_cast<uint32_t>( handleType );

   CYDASSERT( m_activeEntryCount < ( MAX_ENTRIES - 1 ) );

   CYDASSERT( type >= 0 && type <= 31 );

   const int newIndex = m_firstFreeEntry;
   CYDASSERT( newIndex < MAX_ENTRIES );
   CYDASSERT( m_entries[newIndex].active == false );
   CYDASSERT( !m_entries[newIndex].endOfList );

   m_firstFreeEntry                  = m_entries[newIndex].nextFreeIndex;
   m_entries[newIndex].nextFreeIndex = 0;
   m_entries[newIndex].counter       = m_entries[newIndex].counter + 1;
   if( m_entries[newIndex].counter == 0 )
   {
      m_entries[newIndex].counter = 1;
   }
   m_entries[newIndex].active = true;
   m_entries[newIndex].entry  = p;

   ++m_activeEntryCount;

   return Handle( newIndex, m_entries[newIndex].counter, handleType );
}

void HandleManager::update( Handle handle, void* newData )
{
   const int index = handle.index;
   CYDASSERT( m_entries[index].counter == handle.counter );
   CYDASSERT( m_entries[index].active == true );

   m_entries[index].entry = newData;
}

void HandleManager::remove( const Handle handle )
{
   const uint32_t index = handle.index;
   CYDASSERT( m_entries[index].counter == handle.counter );
   CYDASSERT( m_entries[index].active == true );

   m_entries[index].nextFreeIndex = m_firstFreeEntry;
   m_entries[index].active        = 0;
   m_firstFreeEntry               = index;

   --m_activeEntryCount;
}

void* HandleManager::get( Handle handle ) const
{
   void* p = nullptr;
   if( !get( handle, p ) ) return nullptr;
   return p;
}

bool HandleManager::get( const Handle handle, void*& out ) const
{
   const int index = handle.index;
   if( m_entries[index].counter != handle.counter || m_entries[index].active == false )
      return false;

   out = m_entries[index].entry;
   return true;
}
}  // namespace cyd
