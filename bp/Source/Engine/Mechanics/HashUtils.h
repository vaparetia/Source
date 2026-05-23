//----------------------------------------------------------------------------
// HashUtils.h
//
// Hashing functions that are faster/better than CRC.
//
// Bluepoint 2009
//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

namespace HashUtils
{
   // Fast, high quality hashing from Bob Burtle.
   ENGINE_API uint32 HashLittle(void const * pData, uint32 const length, uint32 const initval);
   ENGINE_API uint32 HashLittle(void const * pData, uint32 const length);

   // Hash strings
   ENGINE_API uint32 HashLittle(std::string const &string_);
   ENGINE_API uint32 HashLittle(char const * pString);

   // FNV1a based hashing functions
   ENGINE_API uint32 CalculateFNV1a(void const * pData, int length, uint32 hash = 2166136261u);
   ENGINE_API uint32 CalculateFNV1a(std::string const &s, uint32 hash = 2166136261u);

   // Assumes zero byte terminated string
   ENGINE_API uint32 CalculateAsStringFNV1a(char const* pString, uint32 hash = 2166136261u);
   ENGINE_API uint32 CalculateAsUpperCaseStringFNV1a(char const* pString, uint32 hash = 2166136261u);

};

//----------------------------------------------------------------------------

