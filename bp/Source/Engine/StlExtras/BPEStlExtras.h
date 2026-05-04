#pragma once

#include <math.h>
#include "string"
#include "case_insensitive_char_traits.h"

// std::auto_ptr
#include "memory"

// find
#include "algorithm"

#if BPE_TARGET == BPE_TARGET_X360
#include <ppcintrinsics.h>
#endif

namespace bpe
{
   typedef std::basic_string< char, case_insensitive_char_traits<char> > istring;

   template <class T>
   inline T const min_val(T const &a, T const &b )
   {
     return b < a ? b : a;
   }

   template <class T>
   inline T const max_val(T const &a, T const &b )
   {
     return  a < b ? b : a;
   }

#if BPE_TARGET == BPE_TARGET_X360
   template <>
   inline real32 const min_val( real32 const &a, real32 const &b )
   {
      return fpmin(a, b);
   }

   template <>
   inline real32 const max_val( real32 const &a, real32 const &b  )
   {
      return fpmax(a, b);
   }
#endif

   template <class T>
   inline const T abs(const T& val)
   {
      return (val < 0) ? (-val) : val;
   }

   template <>
   inline const real32 abs( real32 const &val )
   {
      return ::fabsf( val );
   }

   template <>
   inline const real64 abs( real64 const &val )
   {
      return ::fabs( val );
   }

   template <class T>
   inline void clear_keep_capacity(T & val)
   {
      val.erase(val.begin(), val.end());
   }

   template <unsigned N>  
   struct bits_to_store 
   { 
      static int const shifted = N >> 1; 
      static int const value = 1 + bits_to_store< shifted >::value; 
   }; 

   template <> 
   struct bits_to_store<0> 
   { 
      static int const value = 0; 
   };

   template <class T>
   inline void decrement( T &val )
   {
      val = static_cast< T >( static_cast< int >( val ) - 1 );
   }

   template <class T>
   inline void increment( T &val )
   {
      val = static_cast< T >( static_cast< int >( val ) + 1 );
   }
}

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

#include "vector_s.h"
#include "map_s.h"

#include "list"
#include "set"

#include <stdio.h>
#include <stdlib.h>  // malloc etc.

#include "reserved_vector.h"
