//------------------------------------------------------------------------------------------
// TTokenSet.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

template<class T> struct TTokenSet
{
public:
   const char* mString;
   T           mToken;

public:
   T const GetTokenNoCase( std::string const &tokenString ) const    { return GetTokenNoCase(tokenString.c_str()); };
   T const GetTokenNoCase( char const * const pTokenString ) const;
   T const GetToken( std::string const &tokenString ) const          { return GetToken(tokenString.c_str()); };
   T const GetToken( char const * const pTokenString ) const;
   char const * const GetNameByToken( T const token ) const;
};

//------------------------------------------------------------------------------------------

template<class T>
inline T const TTokenSet<T>::GetTokenNoCase( char const * const pTokenString ) const
{
   const TTokenSet<T>*  pCurrentTokenSet;
   
   for ( pCurrentTokenSet = this; pCurrentTokenSet->mString != NULL; ++pCurrentTokenSet )
   {
      if (strcasecmp(pTokenString, pCurrentTokenSet->mString) == 0)
      {
         // Return current
         break;
      }
   }
   // Will return invalid if no match
   return pCurrentTokenSet->mToken;
}

//------------------------------------------------------------------------------------------

template<class T>
inline const T TTokenSet<T>::GetToken( char const * const pTokenString ) const
{
   const TTokenSet<T>*  pCurrentTokenSet;
   
   for ( pCurrentTokenSet = this; pCurrentTokenSet->mString != NULL; ++pCurrentTokenSet )
   {
      if (strcmp(pTokenString, pCurrentTokenSet->mString) == 0)
      {
         // Return current
         break;
      }
   }
   // Will return invalid if no match
   return pCurrentTokenSet->mToken;
}

//------------------------------------------------------------------------------------------

template<class T>
inline char const * const TTokenSet<T>::GetNameByToken( const T token ) const
{
   const TTokenSet<T>* pCurrentTokenSet;

   for( pCurrentTokenSet = this; pCurrentTokenSet->mString != NULL; pCurrentTokenSet++ )
   {
      if ( pCurrentTokenSet->mToken == token )
         return pCurrentTokenSet->mString;
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

