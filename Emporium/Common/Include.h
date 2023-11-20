#pragma once

#include <functional>  // for std::hash

#define REF( EXPR )   \
   do                 \
   {                  \
      sizeof( EXPR ); \
   } while( 0 );

// Forward including handles
#define FWDHANDLE( obj ) typedef struct obj##_T* obj;
#define FWDFLAG( obj ) typedef uint32_t obj;

#define ARRSIZE( obj ) sizeof( obj ) / sizeof( obj[0] )

// Available in C++23
template <typename E>
constexpr auto UNDERLYING( E e ) -> typename std::underlying_type<E>::type
{
   return static_cast<typename std::underlying_type<E>::type>( e );
}

// Add to a class to make it copiable
#define COPIABLE( ClassName )                              \
   ClassName( const ClassName& )                = default; \
   ClassName( ClassName&& ) noexcept            = default; \
   ClassName& operator=( const ClassName& )     = default; \
   ClassName& operator=( ClassName&& ) noexcept = default

// Add to a class to make it non copiable
#define NON_COPIABLE( ClassName )                         \
   ClassName( const ClassName& )                = delete; \
   ClassName( ClassName&& ) noexcept            = delete; \
   ClassName& operator=( const ClassName& )     = delete; \
   ClassName& operator=( ClassName&& ) noexcept = delete

// Add to a class to make it movable
#define MOVABLE( ClassName )                               \
   ClassName( const ClassName& )                = delete;  \
   ClassName( ClassName&& ) noexcept            = default; \
   ClassName& operator=( const ClassName& )     = delete;  \
   ClassName& operator=( ClassName&& ) noexcept = default

// Hashing utility
template <class T>
void hashCombine( size_t& seed, const T& obj )
{
   seed ^= std::hash<T>()( obj ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}
