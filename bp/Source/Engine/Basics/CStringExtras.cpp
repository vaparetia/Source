//----------------------------------------------------------------------------
// CStringExtras.cpp
// Bluepoint
// Copyright 2003
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CStringExtras.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

std::string const CStringExtras::skEmptyString;

//----------------------------------------------------------------------------

std::string const CStringExtras::Stringize( const char * const pFormatString, ... )
{
   char buffer[2048];
   buffer[0] = 0;

   va_list args;
   va_start( args, pFormatString );
#if BPE_TARGET == BPE_TARGET_WIN32
   // Safe version for win32
   vsprintf_s( buffer, sizeof(buffer), pFormatString, args );
#else
   vsprintf( buffer, pFormatString, args );
   BPE_ASSERT(strlen(buffer) < sizeof(buffer), "buffer overrun");
#endif
   va_end( args );

   return string( buffer );
}

//----------------------------------------------------------------------------
// Static version that doesn't do an allocation.
// Make sure you don't store this ptr.
// It's not thread safe either, so watch out.
// Infact, don't use this routine unless you know what you are doing!
char const * const CStringExtras::Stringize_s( const char * const pFormatString, ... )
{
   // MT sanity check
#if _DEBUG
   static int lockCount = 0;
   BPE_ASSERT(lockCount == 0, "CStringExtras::Stringize_s doesn't support MT or re-entrance.");
   lockCount++;
#endif

   // No point putting a CRITICAL_SECTION around this as we have a static buffer.
   // Other threads would be using this buffer outside the CS.
   static char buffer[2048];
   buffer[0] = 0;

   va_list args;
   va_start( args, pFormatString );
#if BPE_TARGET == BPE_TARGET_WIN32
   // Safe version for win32
   vsprintf_s( buffer, sizeof(buffer), pFormatString, args );
#else
   vsprintf( buffer, pFormatString, args );
   BPE_ASSERT(strlen(buffer) < sizeof(buffer), "buffer overrun");
#endif
   va_end( args );

#ifdef _DEBUG
   lockCount--;
#endif

   return buffer;
}
//----------------------------------------------------------------------------
// Static version that doesn't do an allocation.
// Works with %d only and the values have to be less than 100.
// Make sure you don't store this ptr.
// It's not thread safe either, so watch out.
// Infact, don't use this routine unless you know what you are doing!

char const * const CStringExtras::StringizeInt_s( const char * const pFormatString, ... )
{
   // MT sanity check
#if _DEBUG
   static int lockCount = 0;
   BPE_ASSERT(lockCount == 0, "CStringExtras::StringizeInt_s doesn't support MT or re-entrance.");
   lockCount++;
#endif

   // No point putting a CRITICAL_SECTION around this as we have a static buffer.
   // Other threads would be using this buffer outside the CS.
   static char buffer[2048];
   BPE_ASSERT(strlen(pFormatString) < 2048, "Input string too long.");

   va_list args;
   va_start( args, pFormatString );

   // Replace %d with int values
   char const*pCharFS = pFormatString;
   char *pChar = buffer;
   while (*pCharFS != 0)
   {
      if (*pCharFS == '%')
      {
         pCharFS++;
         BPE_ASSERT(*pCharFS == 'd', "Ints only!");

         int const val = va_arg(args, int);
         BPE_ASSERT( val < 100, "2 digit values max");
         if (val < 10)
         {
            *pChar = '0' + val;
         }
         else
         {
            *pChar = '0' + (val / 10);
            pChar++;
            *pChar = '0' + (val % 10);
         }
      }
      else
      {
         *pChar = *pCharFS;
      }
      pCharFS++;
      pChar++;
   }
   *pChar = 0;

   va_end( args );
#ifdef _DEBUG
   lockCount--;
#endif

   return buffer;
}

//----------------------------------------------------------------------------
   
namespace
{
   static inline bool IsDelimiter(char const c, char const * const wstr)
   {
      return (strchr(wstr,c) != NULL);
   }
}

void CStringExtras::Tokenize( std::string const & splitString, 
                              std::vector< std::string > & outTokens,
                              char const * splitCharacters )
{
   string::size_type const S = splitString.size();
   string::size_type i = 0;

   while( i < S )
   {
      // eat leading whitespace
      while( (i < S) && ( IsDelimiter( splitString[i], splitCharacters ) ) )
      {
         ++i;
      }

      if( i == S )
      {
         break;  // nothing left but WS
      }

      // find end of word
      string::size_type j = i+1;
      while ((j < S) && ( !IsDelimiter( splitString[j], splitCharacters ) ) )
      {
         ++j;
      }

      // add word
      outTokens.push_back( splitString.substr( i, j-i ) );

      // set up for next loop
      i = j + 1;
    }
}
   
//----------------------------------------------------------------------------

bool const CStringExtras::CompareCaseInsensitive( std::string const & lhs, std::string const & rhs )
{
   return (strcasecmp(lhs.c_str(), rhs.c_str()) == 0);
}

//----------------------------------------------------------------------------

char *CStringExtras::StrRev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

//----------------------------------------------------------------------------

std::string const CStringExtras::TrimWhitespace( std::string const & input )
{
   string temp = input;
   
   int first = temp.find_first_not_of( " \t\n\r" ); 
   int last = temp.find_last_not_of( " \t\n\r" );
   
   if( first == string::npos || last == string::npos )
      return input;
   
   return temp.substr( first, last - first + 1 );
}

//----------------------------------------------------------------------------

std::string const CStringExtras::SplitLeftLastOf( std::string const & input, char const splitChar )
{
   int const foundPos = input.find_last_of( splitChar );

   if( foundPos != string::npos )
   {
      return input.substr( 0, foundPos );
   }

   return input;
}

//----------------------------------------------------------------------------

std::string const CStringExtras::SplitRightLastOf( std::string const & input, char const splitChar )
{
   int const foundPos = input.find_last_of( splitChar );
   
   if( foundPos != string::npos )
   {
      return input.substr( foundPos + 1 );
   }

   return input;
}

//----------------------------------------------------------------------------

std::string const CStringExtras::FourCCToText( uint32 const fourCC )
{
   string typeText( 5, ' ' );
   
   typeText[0] = toupper( ( fourCC >> 24 ) & 0xff );
   typeText[1] = toupper( ( fourCC >> 16 ) & 0xff );
   typeText[2] = toupper( ( fourCC >> 8  ) & 0xff );
   typeText[3] = toupper( ( fourCC       ) & 0xff );
   typeText[4] = 0;

   return typeText;
}

//----------------------------------------------------------------------------

uint32 const CStringExtras::TextToFourCC( std::string const & text )
{
   return (static_cast<uint32>(text[0]) << 24) |
          (static_cast<uint32>(text[1]) << 16) | 
          (static_cast<uint32>(text[2]) << 8)  | 
          (static_cast<uint32>(text[3]));
}

//----------------------------------------------------------------------------

std::string const CStringExtras::Replace(std::string const & input, std::string const & find, std::string const & replace)
{
   std::string result = input;
   
   int currentPos = 0;

   for(;;)
   {
      int foundPos = result.find(find, currentPos);
      
      if( foundPos == std::string::npos )
         break;
      
      result.replace(foundPos, find.length(), replace);
      
      currentPos = foundPos + replace.length();
   }

   return result;
}
