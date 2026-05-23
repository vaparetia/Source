//----------------------------------------------------------------------------
// CStringExtras.h
// Bluepoint
// Copyright 2003
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "vector"

//----------------------------------------------------------------------------

class ENGINE_API CStringExtras
{
public:
   static std::string const Stringize( const char * const pFormatString, ... );
   static char const * const Stringize_s( const char * const pFormatString, ... );  // Zero allocation, static buffer, no MT, see notes in cpp.
   // Zero allocation, static buffer, no MT, 2 digit int numbers only, see notes in cpp. 
   static char const * const StringizeInt_s( const char * const pFormatString, ... );  

   static bool const CompareCaseInsensitive( std::string const & lhs, std::string const & rhs );
   static char *     StrRev(char * pString);

   static std::string const TrimWhitespace( std::string const & input );

   static std::string const SplitLeftLastOf( std::string const & input, char const splitChar );
   static std::string const SplitRightLastOf( std::string const & input, char const splitChar );

   static void Tokenize( std::string const & splitString, 
                         std::vector< std::string > & outTokens,
                         char const *splitCharacters );

   // convert type to four-character-code string
   static std::string const FourCCToText( uint32 const fourCC );
   // convert four-character-code-string to type
   static uint32 const TextToFourCC( std::string const & text );

   static std::string const Replace(std::string const & input, std::string const & find, std::string const & replace);

   // Useful for passing empty string references for default parameters to improve performance
   static std::string const      skEmptyString;
};

//----------------------------------------------------------------------------

