//----------------------------------------------------------------------------
// CMemoryInputStream.h
// Bluepoint
// Copyright 2001
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/CInputStream.h"

//----------------------------------------------------------------------------

class CMemoryInputStream : public CInputStream
{
public:
   enum EOwner
   {
      kOwner_App,
      kOwner_Stream,

      kOwner_Count
   };

public:
   ENGINE_API CMemoryInputStream(void* pMemory, uint32 const size, EOwner const owner);

protected:
   virtual uint32 const Read(void *pBuffer, uint32 const bufferSize);
};

//----------------------------------------------------------------------------
