//----------------------------------------------------------------------------
// CStringTable.h
// Bluepoint
// Copyright 2008
// Turns an array of strings into single allocation block of strings.
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "vector"

//----------------------------------------------------------------------------

class ENGINE_API CStringTable
{
public:
   ~CStringTable();

   uint32               GetNumStrings() const   { return mNumStrings; }
   uint32               GetAllocSize() const    { return mAllocSize; }
   char const * const   GetString(uint32 const index) const;
   void                 PutTo(COutputStream& stream) const;

   static CStringTable * Factory(std::vector<std::string> const &strings);
   static CStringTable * Factory(void * pMemory);  // Inplace construction, takes ownership of pointer.
   static CStringTable * Factory(CInputStream &stream);   

private:
   uint32   mAllocSize;
   uint32   mNumStrings;
   uint32   mStringOffsets;

   // Use the factories to construct.
   CStringTable();
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CStringTable);
};

