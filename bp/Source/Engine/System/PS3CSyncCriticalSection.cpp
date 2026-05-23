//----------------------------------------------------------------------------
// PS3CSyncCriticalSection.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSyncCriticalSection.h"

#include <sys/synchronization.h>

BPE_CTASSERT(sizeof(BPE_CRITICAL_SECTION) == sizeof(sys_lwmutex_t));

//----------------------------------------------------------------------------

CSyncCriticalSection::CSyncCriticalSection()
{
   sys_lwmutex_attribute_t attrib;
   sys_lwmutex_attribute_initialize(attrib);

   attrib.attr_recursive = SYS_SYNC_RECURSIVE;

   int const ret = sys_lwmutex_create((sys_lwmutex_t*)&mCriticalSection, &attrib);
   BPE_VERIFY(ret == CELL_OK, false, "CSyncCriticalSection - sys_lwmutex_create failed\n");
}

//----------------------------------------------------------------------------

CSyncCriticalSection::~CSyncCriticalSection()
{
   int ret = EBUSY;
   while (ret == EBUSY)
   {
      ret = sys_lwmutex_destroy((sys_lwmutex_t*)&mCriticalSection);
      BPE_VERIFY((ret == CELL_OK) || (ret == EBUSY), false, "CSyncCriticalSection - sys_lwmutex_destroy failed\n");
   }
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Enter()
{
   usecond_t const kTimeout = 0; // infinite
   int const ret = sys_lwmutex_lock((sys_lwmutex_t*)&mCriticalSection, kTimeout);
   BPE_VERIFY(ret == CELL_OK, false, "CSyncCriticalSection::Enter - sys_lwmutex_lock failed\n");
}

//----------------------------------------------------------------------------

bool CSyncCriticalSection::TryEnter()
{
   int const ret = sys_lwmutex_trylock((sys_lwmutex_t*)&mCriticalSection);
   if (ret == EBUSY)
   {
      return false;
   }
   BPE_VERIFY(ret == CELL_OK, false, "CSyncCriticalSection::TryEnter - sys_lwmutex_lock failed\n");
   return true;
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Leave()
{
   int const ret = sys_lwmutex_unlock((sys_lwmutex_t*)&mCriticalSection);
   BPE_VERIFY(ret == CELL_OK, false, "CSyncCriticalSection::Leave - sys_lwmutex_lock failed\n");
}

