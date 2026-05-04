#include "Engine/StdAfx.h"
#include "BP_BuildDefines.h"

#if BP_ENABLE_MEMORY_TRACKING && BPE_TARGET == BPE_TARGET_X360

#include "stdio.h"
#include "xtl.h"
#include "xbdm.h"

int const kMaxStackTraceSize = 32;
int const kMaxAllocTypeCount = 8192;
void* gStackTracePool[kMaxAllocTypeCount][kMaxStackTraceSize];
int gCurrentStackTraceCount = 0;

int GetCallstackId(void* pStackTrace)
{
   for( int i = 0; i < gCurrentStackTraceCount; ++i )
   {
      if( memcmp(&gStackTracePool[i][0], pStackTrace, kMaxStackTraceSize * sizeof(void*)) == 0 )
         return i;
   }

   if( gCurrentStackTraceCount < kMaxAllocTypeCount )
   {
      memcpy(&gStackTracePool[gCurrentStackTraceCount][0], pStackTrace, kMaxStackTraceSize * sizeof(void*));
      gCurrentStackTraceCount++;
      
      return gCurrentStackTraceCount - 1;
   }
   
   __debugbreak();
   return -1;
}

int gAllocationTypeDebugBreak = -1;

int const kMaxAllocationCount = 10240;

struct SAlloc
{
   void* address;
   int type;
   int size;
};

SAlloc gAllocations[kMaxAllocationCount];
int gCurrentAllocationCount = 0;

void AddAlloc(void* address, int type, int size)
{
   if( type == gAllocationTypeDebugBreak )
      __debugbreak();

   SAlloc * pAllocation = NULL;

   for( int i = 0; i < gCurrentAllocationCount; ++i )
   {
      if( gAllocations[i].address == NULL )
      {
         pAllocation = gAllocations + i;
         break;
      }
   }

   if( pAllocation == NULL && gCurrentAllocationCount < kMaxAllocationCount )
   {
      pAllocation = gAllocations + gCurrentAllocationCount;
      ++gCurrentAllocationCount;
   }

   if( pAllocation == NULL )
      __debugbreak();

   pAllocation->address = address;
   pAllocation->type = type;
   pAllocation->size = size;
}

void RemoveAlloc(void* address)
{
   for( int i = 0; i < gCurrentAllocationCount; ++i )
   {
      SAlloc & alloc = gAllocations[i];

      if( alloc.address == address )
      {
         memset(&alloc, 0, sizeof(SAlloc));
         break;
      }
   }

   for( ; gCurrentAllocationCount > 0; --gCurrentAllocationCount )
   {
      SAlloc & alloc = gAllocations[gCurrentAllocationCount - 1];
      
      if( alloc.address != NULL )
         break;
   }
}

void DumpMemoryStats()
{
   printf("Memory Stats:\n");

   for( int allocType = 0; allocType < gCurrentStackTraceCount; ++allocType )
   {
      int totalAllocationSize = 0;
      int totalAllocationCount = 0;

      for( int j = 0; j < gCurrentAllocationCount; ++j )
      {
         SAlloc const & alloc = gAllocations[j];
         if( alloc.type == allocType && alloc.address != NULL)
         {
            totalAllocationSize += alloc.size;
            totalAllocationCount++;
         }
      }

      if( totalAllocationCount > 0 )
      {
         printf("AllocType[%d]: Count: %d Size: %d\n", allocType, totalAllocationCount, totalAllocationSize);
      }
   }
}

VOID* WINAPI XMemAlloc( SIZE_T size, DWORD dwAllocAttributes )
{
   void* gStackTrace[kMaxStackTraceSize];
   DmCaptureStackBackTrace(kMaxStackTraceSize, (PVOID*)&gStackTrace);

   int callStackId = GetCallstackId(gStackTrace);
   VOID* result = XMemAllocDefault(size, dwAllocAttributes);
   
   AddAlloc(result, callStackId, size);

   return result;
}

VOID WINAPI XMemFree( VOID* pMemory, DWORD dwAllocAttributes )
{
   RemoveAlloc(pMemory);

   XMemFreeDefault(pMemory, dwAllocAttributes);
}

SIZE_T WINAPI XMemSize( VOID* pAddress, DWORD dwAllocAttributes )
{
   return XMemSizeDefault(pAddress, dwAllocAttributes);
}

#endif