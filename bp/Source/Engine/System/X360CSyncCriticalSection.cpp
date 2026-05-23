//----------------------------------------------------------------------------
// X360CSyncCriticalSection.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include "CSyncCriticalSection.h"

//----------------------------------------------------------------------------

CSyncCriticalSection::CSyncCriticalSection()
{
   InitializeCriticalSection(&mCriticalSection);
}

//----------------------------------------------------------------------------

CSyncCriticalSection::~CSyncCriticalSection()
{
   DeleteCriticalSection(&mCriticalSection);
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Enter()
{
   EnterCriticalSection(&mCriticalSection);

}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Leave()
{
   LeaveCriticalSection(&mCriticalSection);

}

//----------------------------------------------------------------------------

bool CSyncCriticalSection::TryEnter()
{
   return TryEnterCriticalSection(&mCriticalSection) == TRUE;
}

