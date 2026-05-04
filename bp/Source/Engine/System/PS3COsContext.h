//----------------------------------------------------------------------------
// PS3COsContext.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "COsContext.h"

//----------------------------------------------------------------------------

class ENGINE_API COsContext : public CBaseOsContext
{
public:
   COsContext();
   ~COsContext();

public:
   BPE_HANDLE              mInstanceHandle;
   BPE_HANDLE              mMainWindow;

   virtual void            ProcessPlatform();

   sys_memory_container_t  LockSysMemoryContainer(size_t const size);
   void                    UnlockSysMemoryContainer();
   size_t                  GetSysMemoryContainerMaxSize() const;
   void                    DestroyMemoryContainer();
   void                    ReinitializeNp();
   uint8 *                 AllocateTempMemory(int32 const size);  // Allocates some temporary memory from the sys memory container. Transient usage only, can fail.
   void                    DeallocateTempMemory(uint8 * pMemory);

   // Launch new executable, using relative path, doesn't call destructors, handles NPDRM and patch SELFs (path from disc or system root, no starting '\')
   void                    _Exitspawn(char const * const pRelativePath, char *argv[]) const;
   // Load SPRX wrapper (handles NPDRM)
   int                     sys_prx_load_module(char const * const pRelativePath) const;

   // Displays the cellGameContentErrorDialog
   static void             TerminateLowDiskSpace(int needSizeKB);
   static void             TerminateLicenseCheckFailed();
   static void             WrongUserWarning(int *pResult);
   static void             CorruptSaveWarning(int *pResult);

   // Misc cellGameBootCheck data
   int32                   mHDDFreeSizeKB;
   uint32                  mGameBootType;
   uint32                  mGameBootAttributes;
   char                    mGameContentInfoPath[128];
   char                    mGameUsrdirPath[128];
   // cellGamePathCheck data
   char                    mPatchContentInfoPath[128];
   char                    mPatchUsrdirPath[128];

private:
   static size_t const     skNPInitPoolSize;
   void *                  mpNpInitPoolAlloc;
   static size_t const     skHTTPInitPoolSize;
   void *                  mpHTTPInitPoolAlloc;

   // 64Meg Memory container allocated globally at start to prevent allocation failure
   static size_t const     skMemoryContainerSize;
   sys_memory_container_t  mSysMemoryContainer;
   int32                   mSysMemoryContainerRefCount;


   // Misc initialization functions
   void                    InitializeBuildSKU();
   void                    InitializeLanguage();
};

//----------------------------------------------------------------------------

BPE_FORCEINLINE COsContext * OsContext()
{
   return static_cast<COsContext*>(gpOsContext);
}

//----------------------------------------------------------------------------

