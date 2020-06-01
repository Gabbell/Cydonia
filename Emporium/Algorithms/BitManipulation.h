#pragma once

#include <vector>

namespace EMP
{
template <typename T>
void BitReversalPermutation( std::vector<T>& values )
{
   // Bit reversal only valid for integral types for now
   static_assert( std::is_integral_v<T> );

   for( uint32_t i = 0, j = 0; i < values.size(); i++ )
   {
      if( i < j )
      {
         std::swap( values[i], values[j] );
      }

      // Compute a mask of LSBs.
      const uint32_t mask = i ^ ( i + 1 );

      // Using division to bit-reverse a single bit.
      const uint32_t rev = static_cast<uint32_t>( values.size() ) / ( mask + 1 );

      // XOR with mask.
      j ^= values.size() - rev;
   }
}
}
