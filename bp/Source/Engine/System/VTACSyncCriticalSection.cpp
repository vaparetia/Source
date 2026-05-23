//----------------------------------------------------------------------------
// VTACSyncCriticalSection.cpp
// Armature
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSyncCriticalSection.h"

#include <kernel.h>

//----------------------------------------------------------------------------

CSyncCriticalSection::CSyncCriticalSection()
{
   SceInt32 res = sceKernelCreateLwMutex((SceKernelLwMutexWork*)&mCriticalSection, "bpe_mutex", 
      SCE_KERNEL_LW_MUTEX_ATTR_TH_PRIO | SCE_KERNEL_LW_MUTEX_ATTR_RECURSIVE, 0, NULL);

   BPE_VERIFY(res == SCE_OK, false, "CSyncCriticalSection - CreateLwMutex failed.");
}

//----------------------------------------------------------------------------

CSyncCriticalSection::~CSyncCriticalSection()
{
   SceInt32 res = sceKernelDeleteLwMutex((SceKernelLwMutexWork*)&mCriticalSection);
   BPE_VERIFY(res == SCE_OK, false, "CSyncCriticalSection - DeleteLwMutex failed.");
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Enter()
{
   // Specify a lock count greater than 1 since we created the mutex with the recursive attribute.
   SceInt32 res = sceKernelLockLwMutex((SceKernelLwMutexWork*)&mCriticalSection, 2, NULL);
   BPE_VERIFY(res == SCE_OK, false, "CSyncCriticalSection - LockLwMutex failed.");
}

//----------------------------------------------------------------------------

bool CSyncCriticalSection::TryEnter()
{
   SceInt32 res = sceKernelTryLockLwMutex((SceKernelLwMutexWork*)&mCriticalSection, 2);
   return (res == SCE_OK);
}

//----------------------------------------------------------------------------

void CSyncCriticalSection::Leave()
{
   SceInt32 res = sceKernelUnlockLwMutex((SceKernelLwMutexWork*)&mCriticalSection, 2);
   BPE_VERIFY(res == SCE_OK, false, "UnlockLwMutex failed.");
}

