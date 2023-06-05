#pragma once

#include <cstdio>

#if CYD_ASSERTIONS_ENABLED
#define CYDASSERT( EXPR )                                                       \
   if( !( EXPR ) )                                                              \
   {                                                                            \
      fprintf(                                                                  \
          stderr,                                                               \
          "Assert Failed->\n \tFile: %s\n \tLine: %d\n \tPrecondition: (%s)\n", \
          __FILE__,                                                             \
          __LINE__,                                                             \
          #EXPR );                                                              \
      __debugbreak();                                                           \
   }
#else
#define CYDASSERT( EXPR ) \
   do                     \
   {                      \
      sizeof( EXPR );     \
   } while( 0 );
#endif

#if CYD_ASSERTIONS_ENABLED
#define CYDASSERT_AND_RETURN( EXPR, RET )                                       \
   if( !( EXPR ) )                                                              \
   {                                                                            \
      fprintf(                                                                  \
          stderr,                                                               \
          "Assert Failed->\n \tFile: %s\n \tLine: %d\n \tPrecondition: (%s)\n", \
          __FILE__,                                                             \
          __LINE__,                                                             \
          #EXPR );                                                              \
      __debugbreak();                                                           \
      RET;                                                                      \
   }
#else
#define CYDASSERT_AND_RETURN( EXPR, RET ) \
   do                                     \
   {                                      \
      sizeof( EXPR );                     \
   } while( 0 );                          \
   RET;
#endif
