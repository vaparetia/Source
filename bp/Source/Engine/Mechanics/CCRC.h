//----------------------------------------------------------------------------
// CCRC.h
// Copyright 2006
//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class ENGINE_API CCRC
{
public:
   static uint32 CalculateCRC32(void const* pData, int length);
   static uint32 CalculateCRC32(std::string const &s);

   // Assumes zero byte terminated string
   static uint32 CalculateCRC32AsString(char const* pString);
   static uint32 CalculateCRC32AsUpperCaseString(char const* pString);
};

//----------------------------------------------------------------------------

