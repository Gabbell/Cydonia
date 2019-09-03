#pragma once

// Cydonia Asserts
// Available only in debug

#if defined( _DEBUG )
#define CYDASSERT( EXPR )                                                        \
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
   do                    \
   {                     \
      sizeof( EXPR );    \
   } while( 0 );
#endif
