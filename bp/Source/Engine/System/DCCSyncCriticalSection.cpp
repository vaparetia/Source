//----------------------------------------------------------------------------
// DCCSyncCriticalSection.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSyncCriticalSection.h"

#include <kos/mutex.h>

//----------------------------------------------------------------------------

CSyncCriticalSection::CSyncCriticalSection()
{
   mutex_init(&mCriticalSection, MUTEX_TYPE_RECURSIVE);
}

//----------------------------------------------------------------------------

CSyncCriticalSection::~CSyncCriticalSection()
{
   mutex_destroy(&mCriticalSection);
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Enter()
{
   mutex_lock(&mCriticalSection);
}

//----------------------------------------------------------------------------

bool CSyncCriticalSection::TryEnter()
{
   return mutex_trylock(&mCriticalSection) == 0;
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Leave()
{
   mutex_unlock(&mCriticalSection);
}

//----------------------------------------------------------------------------
