//----------------------------------------------------------------------------
// VTACOsContext.h
// Armature
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "COsContext.h"

#include <fios2/fios2_all.h>

#include <kernel/libkernel.h>
#include <sdk_version.h>
#include <apputil.h>

//----------------------------------------------------------------------------

class ENGINE_API COsContext : public CBaseOsContext
{
public:
   COsContext();
   ~COsContext();

public:
   // Misc initialization functions
   void                    InitializeBuildSKU();
   void                    InitializeLanguage();
   void                    InitializeLowLevelLanguage();

   static char const *GetRootFolder();

   // Gets the number of times the system has been resumed from suspension.
   // This count monotonically increases over time, so if you want to see if you've
   // been suspended, cache this count and check against it.
   int                     GetSystemResumeCount() const { return mSystemResumeCount; }

   virtual void            ProcessPlatform();

private:
   void                    InitializeNetwork();
   void                    InitializeBuildVersion();
   void                    InitializeFios();
   void                    InitializeAppUtil();
   void                    InitializeCommonDialogConfig();
   void                    InitializeBGMConfig();

   // FIOS - Note that this should really be somewhere else

   void                    ShutdownFios();

   static const size_t MAX_PATH_LENGTH = 1024;
   static const size_t skMaxFiosOps = 64;
   static const size_t skMaxFiosChunks = 1024;
   static const size_t skMaxFiosFileHandles = 64;
   static const size_t skMaxFiosDirHandles = 4;

   SceFiosPsarcDearchiverContext mDearchiverContext;
   char mOpStorage[ SCE_FIOS_OP_STORAGE_SIZE(skMaxFiosOps, MAX_PATH_LENGTH) ] __attribute__((aligned(8)));
   char mChunkStorage[ SCE_FIOS_CHUNK_STORAGE_SIZE(skMaxFiosChunks) ] __attribute__((aligned(8)));
   char mFHStorage[ SCE_FIOS_FH_STORAGE_SIZE(skMaxFiosFileHandles, MAX_PATH_LENGTH) ] __attribute__((aligned(8)));
   char mDHStorage[ SCE_FIOS_DH_STORAGE_SIZE(skMaxFiosDirHandles, MAX_PATH_LENGTH) ] __attribute__((aligned(8)));

   bool TryMountArchive( char const *file, char const *mountPoint );

   SceFiosFH mMountFileHandle;
   void *mpMountBuffer;

   SceAppUtilInitParam mAppUtilInitParam;
   SceAppUtilBootParam mAppUtilBootParam;

   SceInt32 mLowLevelLanguage;

   int mSystemResumeCount;
};

//----------------------------------------------------------------------------

BPE_FORCEINLINE COsContext * OsContext()
{
   return static_cast<COsContext*>(gpOsContext);
}

//----------------------------------------------------------------------------

