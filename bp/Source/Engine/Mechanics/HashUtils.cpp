//----------------------------------------------------------------------------
// HashUtils.h
//
// Hashing functions that are faster/better than CRC.
//
// Bluepoint 2009
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "HashUtils.h"
#include "ctype.h"
//----------------------------------------------------------------------------

namespace HashUtils
{

//----------------------------------------------------------------------------

// Typedefs for lookup3.c
#ifndef uint32_t

typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed int      int32_t;
typedef unsigned int    uint32_t;
typedef signed int      intptr_t;
typedef unsigned int    uintptr_t;

#endif

// See http://burtleburtle.net/bob/hash/doobs.html for details.
#include "Engine/Mechanics/burtle/lookup3.c"

//----------------------------------------------------------------------------

uint32 HashLittle(void const * pData, uint32 const length, uint32 const initval)
{
   return hashlittle(pData, length, initval);
}

//----------------------------------------------------------------------------

uint32 HashLittle(void const * pData, uint32 const length)
{
   return hashlittle(pData, length, 0);
}

//----------------------------------------------------------------------------

uint32 HashLittle(std::string const &string_)
{
   return hashlittle(string_.c_str(), string_.size(), 0);
}

//----------------------------------------------------------------------------

uint32 HashLittle(char const * pString)
{
   // Ugh! Would nice to have a zero terminated version of hashlittle!
   return hashlittle(pString, strlen(pString), 0);
}

//----------------------------------------------------------------------------

uint32 CalculateFNV1a(void const * pData, int length, uint32 hash)
{
   const unsigned char *d = static_cast<const unsigned char *>(pData);
   const unsigned char *e = d + length;
   while (d < e)
   {
      hash ^= *d++;
      hash *= 16777619u;
   }
   return hash;
}

//----------------------------------------------------------------------------

uint32 CalculateFNV1a(std::string const &s, uint32 hash)
{
   return CalculateFNV1a(s.c_str(), s.size(), hash);
}

//----------------------------------------------------------------------------

// Assumes zero byte terminated string
uint32 CalculateAsStringFNV1a(char const* pString, uint32 hash)
{
   if (pString == 0)
      return 0;
   for (const char *s = pString; *s != 0; ++s)
   {
      hash ^= *s;
      hash *= 16777619u;
   }
   return hash;
}

//----------------------------------------------------------------------------

uint32 CalculateAsUpperCaseStringFNV1a(char const* pString, uint32 hash)
{
   if (pString == 0)
      return 0;
   for (const char *s = pString; *s != 0; ++s)
   {
      hash ^= static_cast<unsigned char>(toupper(*s));
      hash *= 16777619u;
   }
   return hash;
}

//----------------------------------------------------------------------------

} // namespace HashUtils;

