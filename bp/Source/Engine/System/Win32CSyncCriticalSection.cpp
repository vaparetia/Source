//----------------------------------------------------------------------------
// Win32CSyncCriticalSection.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSyncCriticalSection.h"

#include <windows.h>
BPE_CTASSERT(sizeof(BPE_CRITICAL_SECTION) == sizeof(CRITICAL_SECTION));

//----------------------------------------------------------------------------

CSyncCriticalSection::CSyncCriticalSection()
{
   InitializeCriticalSection((LPCRITICAL_SECTION) &mCriticalSection);
}

//----------------------------------------------------------------------------

CSyncCriticalSection::~CSyncCriticalSection()
{
   DeleteCriticalSection((LPCRITICAL_SECTION) &mCriticalSection);
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Enter()
{
   EnterCriticalSection((LPCRITICAL_SECTION) &mCriticalSection);
}

//----------------------------------------------------------------------------

bool CSyncCriticalSection::TryEnter()
{
   return TryEnterCriticalSection((LPCRITICAL_SECTION) &mCriticalSection) == TRUE;
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Leave()
{
   LeaveCriticalSection((LPCRITICAL_SECTION) &mCriticalSection);
}



