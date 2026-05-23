//----------------------------------------------------------------------------
// CMemoryInputStream.cpp
// Bluepoint
// Copyright 2001
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Streams/CMemoryInputStream.h"

//----------------------------------------------------------------------------

CMemoryInputStream::CMemoryInputStream(void* pMemory, const uint32 size, EOwner const owner)
:  CInputStream(pMemory, size, (owner == kOwner_Stream))
{
}

//----------------------------------------------------------------------------

uint32 const CMemoryInputStream::Read(void *pBuffer, uint32 const bufferSize)
{
   return 0;
}

//----------------------------------------------------------------------------
