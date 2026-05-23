//----------------------------------------------------------------------------
// CSyncCriticalSection.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#if BPE_TARGET==BPE_TARGET_X360
#include <xtl.h>
#endif

//----------------------------------------------------------------------------

class ENGINE_API CSyncCriticalSection
{
public:
   CSyncCriticalSection();
   ~CSyncCriticalSection();

   void Enter();
   bool TryEnter();
   void Leave();

private:
   BPE_CRITICAL_SECTION  mCriticalSection;
};

//----------------------------------------------------------------------------

class CSyncCriticalSectionLocker
{
public:
   CSyncCriticalSectionLocker(CSyncCriticalSection & criticalSection)
   :  mpCriticalSection(&criticalSection)
   {
      if (mpCriticalSection)
      {
         mpCriticalSection->Enter();
      }
   }

   // Optional lock
   CSyncCriticalSectionLocker(CSyncCriticalSection * pCriticalSection)
   :  mpCriticalSection(pCriticalSection)
   {
      if (mpCriticalSection)
      {
         mpCriticalSection->Enter();
      }
   }
   
   ~CSyncCriticalSectionLocker()
   {
      if (mpCriticalSection)
      {
         mpCriticalSection->Leave();
      }
   }

private:
   CSyncCriticalSection * mpCriticalSection;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CSyncCriticalSectionLocker);
};

//----------------------------------------------------------------------------
#if BPE_TARGET != BPE_TARGET_WIN32

// PS3/RVL doesn't need critical sections as we're not using multiple threads for object thinks
#define BPE_CRITCAL_SECTION_LOCK_MT_PS(_criticalSection) ((void)0)

#else

// For platforms that use multiple threads for object thinks
#define BPE_CRITCAL_SECTION_LOCK_MT_PS(_criticalSection) CSyncCriticalSectionLocker _csLock(_criticalSection)

#endif

// Lock a critical section for this scope
#define MTX_CRITCAL_SECTION_LOCK_SCOPE(_criticalSection) CSyncCriticalSectionLocker _csLock(_criticalSection)

//----------------------------------------------------------------------------
