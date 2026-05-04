//----------------------------------------------------------------------------
// RVLCSyncCriticalSection.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSyncCriticalSection.h"

//----------------------------------------------------------------------------

CSyncCriticalSection::CSyncCriticalSection()
{
   OSInitMutex( &mCriticalSection );
}

//----------------------------------------------------------------------------

CSyncCriticalSection::~CSyncCriticalSection()
{
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Enter()
{
   OSLockMutex( &mCriticalSection );
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Leave()
{
   OSUnlockMutex( &mCriticalSection );
}

//----------------------------------------------------------------------------

bool CSyncCriticalSection::TryEnter()
{
   return OSTryLockMutex( &mCriticalSection );
}

