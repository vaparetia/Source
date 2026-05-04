#include "Engine/Memory/VTAPhysContMemory.h"

#include <kernel.h>
#include <kernel/sysmem.h>
#include <sceerror.h>
#include <stdlib.h>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/System/CSyncCriticalSection.h"

static SceUID gsPhysContMemBlock;
static void* gspPhysContMemStartAddr;
static CSyncCriticalSection gsMemMutex;
static bool gbIsMemLocked;
static NPhysContMem::EMemOwnerType gsLastLockedOwnerType;
static NPhysContMem::FORCE_UNLOCK_CALLBACK gspLastLockedOwnerUnlockCallback;
static void* gspLastLockedOwnerCallbackUserData;

//----------------------------------------------------------------------------
static void free_phys_cont_mem_block()
{
   if (!gspPhysContMemStartAddr)
      return;
   
   int res = sceKernelFreeMemBlock(gsPhysContMemBlock);
   BPE_VERIFY(res == SCE_OK, false, "Error freeing memory block.");

   gsPhysContMemBlock = SCE_UID_INVALID_UID;
   gspPhysContMemStartAddr = NULL;
}

//----------------------------------------------------------------------------
void NPhysContMem::InitPhysContMem()
{
   gsPhysContMemBlock = SCE_UID_INVALID_UID;
   gspPhysContMemStartAddr = NULL;
   gbIsMemLocked = false;
   gsLastLockedOwnerType = OWNER_TYPE_TEXTURE_DECOMPRESSION;
   gspLastLockedOwnerUnlockCallback = NULL;
   gspLastLockedOwnerCallbackUserData = NULL;
}

//----------------------------------------------------------------------------
void* NPhysContMem::LockPhysContMem(NPhysContMem::EMemOwnerType ownerType, FORCE_UNLOCK_CALLBACK pUnlockCallback, 
   void* pUserData)
{
   CSyncCriticalSectionLocker lock(gsMemMutex);

   // If a system other than texture decompression currently has the memory locked, unlock it since policy states
   // texture decompression trumps other systems.
   if (ownerType == OWNER_TYPE_TEXTURE_DECOMPRESSION)
   {
      if (gbIsMemLocked)
      {
         if (gspLastLockedOwnerUnlockCallback)
            gspLastLockedOwnerUnlockCallback(gspLastLockedOwnerCallbackUserData);

         UnlockPhysContMem();
      }
   }

   BPE_VERIFY(!gbIsMemLocked, false, "Memory is currently locked and cannot be accessed!");

   // Only free and reallocate the memory if the owner type differs from the last locked owner type. This is to prevent
   // multiple allocations/frees from occurring if the same system locks/unlocks repeatedly as is the case with 
   // decompressing a bunch of textures in a row.
   if (ownerType != gsLastLockedOwnerType || gsPhysContMemBlock == SCE_UID_INVALID_UID)
   {
      free_phys_cont_mem_block();

      BPE_VERIFY(gsPhysContMemBlock == SCE_UID_INVALID_UID, false, "Memory should not be allocated.");

      // Allocate all available physically continuous memory if we are doing texture decompression. If the movie player
      // is locking the memory, don't do any allocations since it needs to handle allocations itself.
      if (ownerType == OWNER_TYPE_TEXTURE_DECOMPRESSION)
      {
         gsPhysContMemBlock = sceKernelAllocMemBlock("scratchpad_cont_mem", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW,
            NPhysContMem::kPhysContMemAllocSize, NULL);
         BPE_VERIFY(gsPhysContMemBlock > 0, false, "Could not allocate physically continuous memory block.");

         SceInt32 res = sceKernelGetMemBlockBase(gsPhysContMemBlock, &gspPhysContMemStartAddr);
         BPE_VERIFY(res == SCE_OK, false, "Could not physically continuous memory block address.");
      }
   }

   gbIsMemLocked = true;
   gsLastLockedOwnerType = ownerType;
   gspLastLockedOwnerCallbackUserData = pUserData;
   gspLastLockedOwnerUnlockCallback = pUnlockCallback;

   return gspPhysContMemStartAddr;
}

//----------------------------------------------------------------------------
void NPhysContMem::UnlockPhysContMem()
{
   CSyncCriticalSectionLocker lock(gsMemMutex);
   gbIsMemLocked = false;
}

//----------------------------------------------------------------------------
void NPhysContMem::ShutdownPhysContMem()
{
   free_phys_cont_mem_block();
}
