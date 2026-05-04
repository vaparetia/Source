#pragma once

namespace bpe
{
   template <class E> struct case_insensitive_char_traits
   {
      typedef E char_type;
      typedef int int_type;
      typedef int pos_type;
      typedef int off_type;
      typedef int state_type;

      static void assign(E& x, const E& y)
      {
         x = y;
      }

      static E *assign(E *x, size_t n, const E& y)
      {
         for( size_t loop = 0; loop < n; ++loop )
         {
            x[loop] = y;
         }

         return x;
      }

      static bool eq(const E& x, const E& y)
      {
         return ( tolower(x) == tolower(y) );
      }

      static bool lt(const E& x, const E& y)
      {
         return tolower(x) < tolower(y);
      }

      static int compare(const E *x, const E *y, size_t n)
      {
         return strncasecmp( x, y, n );
      }

      static size_t length(const E *x)
      {
         const char_type* it = x;
      
         while( *it != 0 )
         {
             ++it;
         }

         return it - x;
      }

      static E *copy(E *x, const E *y, size_t n)
      {
         for( size_t loop = 0; loop < n; ++loop )
         {
            x[loop] = y[loop];
         }
         return x;
      }

      static E *move(E *s1, const E *s2, size_t n)
      {
         E* r = s1;
         if (s1 < s2)
         {
            for (size_t i = 0; i < n; ++i)
               assign(*s1++, *s2++);
         }
         else if (s2 < s1)
         {
            s1 += n;
            s2 += n;
            for (; n > 0; --n)
               assign(*--s1, *--s2);
         }
         return r;
      }

      static const E *find(const E *x, size_t n, const E& y)
      {
         for( size_t loop = 0; loop < n; ++loop )
         {
            if( eq( x[loop], y ) )
            {
               return x + loop;
            }
         }
         return NULL;
      }

      static E to_char_type(const int_type& ch)
      {
         return static_cast<char_type>( ch );
      }

      static int_type to_int_type(const E& c)
      {
         return static_cast<int_type>( c );
      }

      static bool eq_int_type(const int_type& ch1, const int_type& ch2)
      {
         return ch1 == ch2;
      }

      static int_type eof()
      {
         return -1;
      }

      static int_type not_eof(const int_type& ch)
      {
         return ( ch == eof() ? 0 : ch );
      }
   };
}

