//----------------------------------------------------------------------------
// BP_RenderBuffer.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"
#include "BP_RenderBuffer.h"
#include "BP_Debug.h"
#include "Engine/System/CSyncCriticalSection.h"
#include "BP_Memory.h"
#include "BP_Renderer.h"

#include "Renderer/Base/Backend/CRenderBackend.h"

#if BP_VITA
#  include <kernel/threadmgr.h>
#  define VITA_TRACKS_WRITER_THREAD 1
#endif

//----------------------------------------------------------------------------

struct SRenderBuffer
{
   SRenderBuffer()
   {
      mLockedForRead = false;
   }

   char*             gpRenderBufferStart;
   char*             gpRenderBufferEnd;

   char*             gpRenderBufferCurrent;

   SRenderCommand*   gpRenderCommandHead;
   SRenderCommand*   gpRenderCommandTail;

   bool              mLockedForRead;

   int               mPeakSize;
   char              mPeakArea[256];
};

int gBP_RB_WritableBuffer = 0;
int gBP_RB_ReadlockedBuffer = 0;

namespace
{
   SRenderBuffer     gRenderBuffers[2];

   // gpWritableBuffers[0] is always NULL
   // gpWritableBuffers[1] is the current writable buffer, if there is one.
   //   it will be NULL is the buffer we want to write to is locked for read.
   SRenderBuffer    *gpWritableBuffers[ 2 ] = { 0 };

   // sCurrentThread is 1 is we're a valid writer thread, and 0 otherwise
   // This gets handled by Vita's TLS system.
   // On non-threaded platofrms, sCurrentThread is always 1
#if VITA_TRACKS_WRITER_THREAD
   int __thread sCurrentThread = 0;
#else
   int const sCurrentThread = 1;
#endif

   CSyncCriticalSection sWritableBufferMutex;

   // Gives you the current valid writable buffer.  If you're not in a writable thread, this will give you
   // a NULL object and a nice crash.
   // Also, if you're trying to write to a read-locked buffer, you'll get a crash from a NULL deref.
   inline SRenderBuffer& GetWritableRenderBuffer() { return *( gpWritableBuffers[ sCurrentThread ] ); }

   void RecacheWritableRenderBuffer()
   {
      // This function recaches the structure we use to enforce writability.
      // Basically, if our writable buffer is locked for read, then our cached buffer is null

      CSyncCriticalSectionLocker locker( &sWritableBufferMutex );

      if ( gRenderBuffers[ gBP_RB_WritableBuffer ].mLockedForRead )
      {
         gpWritableBuffers[1] = NULL;
      }
      else
      {
         gpWritableBuffers[1] = gRenderBuffers + gBP_RB_WritableBuffer;
      }
   }

   void SetCurrentWritableRenderBufferIndex( int index )
   {
      gBP_RB_WritableBuffer = index;
      RecacheWritableRenderBuffer();
   }

   inline void verify_current_thread()
   {
      BPE_VERIFY( sCurrentThread == 1, false, "RenderBuffer - Writable thread not current" );
   }

   inline void verify_writable_buffer_not_readlocked()
   {
      BPE_VERIFY( !gRenderBuffers[ gBP_RB_WritableBuffer ].mLockedForRead, false, "RenderBuffer - Locked for read" );
   }

   int sLogDepth = 0;
   unsigned int sLogLastId = 0xFFFFFFFF;
   int sLogRepeatCount = 0;

}

//----------------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------------

void BP_RB_Init(int bufferSize)
{
   BP_RB_SetCurrentWriterThread();

   for( int i = 0; i < 2; ++i )
   {
      gRenderBuffers[i].gpRenderBufferStart = gRenderBuffers[i].gpRenderBufferCurrent = (char*) BP_Memory_Alloc( bufferSize, 128, kMT_Permanent, kMC_Renderer );
      gRenderBuffers[i].gpRenderBufferEnd = gRenderBuffers[i].gpRenderBufferStart + bufferSize;
      gRenderBuffers[i].mPeakSize = 0;

      BP_RB_SetCurrentBufferIndex(i);
      BP_RB_Clear();
   }
   
   BP_RB_SetCurrentBufferIndex(0);
   
   BP_RB_UnsetCurrentWriterThread();
}

//----------------------------------------------------------------------------

void BP_RB_LockBufferForRead( int bufferIndex )
{
   gRenderBuffers[bufferIndex].mLockedForRead = true;
   RecacheWritableRenderBuffer();
   gBP_RB_ReadlockedBuffer = bufferIndex;
}

//----------------------------------------------------------------------------

void BP_RB_UnlockBufferForRead( int bufferIndex )
{
   gRenderBuffers[bufferIndex].mLockedForRead = false;
   RecacheWritableRenderBuffer();
   gBP_RB_ReadlockedBuffer = -1;
}

//----------------------------------------------------------------------------

void BP_RB_SetCurrentWriterThread()
{
#if VITA_TRACKS_WRITER_THREAD
   sCurrentThread = 1;
#endif

   RecacheWritableRenderBuffer();
}

//----------------------------------------------------------------------------

void BP_RB_UnsetCurrentWriterThread()
{
#if VITA_TRACKS_WRITER_THREAD
   sCurrentThread = 0;
#endif

   RecacheWritableRenderBuffer();
}

//----------------------------------------------------------------------------

int BP_RB_DebugGetUsedSize()
{
   // This is a debug function, so we don't care about thread safety or whatnots
   int maxSize = 0;

   for ( int i = 0; i < BPE_ARRAY_SIZE( gRenderBuffers ); ++i )
   {
      maxSize = bpe::max_val( maxSize, gRenderBuffers[i].gpRenderBufferCurrent - gRenderBuffers[i].gpRenderBufferStart );
   }

   return maxSize;
}

//----------------------------------------------------------------------------

int BP_RB_DebugGetTotalSize()
{
   // This is a debug function, so we don't care about thread safety or whatnots
   int maxSize = 0;

   for ( int i = 0; i < BPE_ARRAY_SIZE( gRenderBuffers ); ++i )
   {
      maxSize = bpe::max_val( maxSize, gRenderBuffers[i].gpRenderBufferEnd - gRenderBuffers[i].gpRenderBufferStart );
   }

   return maxSize;
}

//----------------------------------------------------------------------------

int BP_RB_DebugGetPeakSize()
{
   // This is a debug function, so we don't care about thread safety or whatnots
   int peakSize = 0;
   for ( int i = 0; i < BPE_ARRAY_SIZE( gRenderBuffers ); ++i )
   {
      peakSize = bpe::max_val( peakSize, gRenderBuffers[i].mPeakSize );
   }
   return peakSize;
}

//----------------------------------------------------------------------------

char * BP_RB_DebugGetPeakArea()
{
   // This is a debug function, so we don't care about thread safety or whatnots
   char *peakArea = "";
   int peakSize = 0;
   for ( int i = 0; i < BPE_ARRAY_SIZE( gRenderBuffers ); ++i )
   {
      if (gRenderBuffers[i].mPeakSize > peakSize)
      {
         peakSize = gRenderBuffers[i].mPeakSize;
         peakArea = gRenderBuffers[i].mPeakArea;
      }
   }
   return peakArea;
}

//----------------------------------------------------------------------------

void BP_RB_SetCurrentBufferIndex(int bufferIndex)
{
   verify_current_thread();

   gBP_RB_WritableBuffer = bufferIndex;
   RecacheWritableRenderBuffer();

   verify_writable_buffer_not_readlocked();
}

//----------------------------------------------------------------------------

void BP_RB_Clear()
{
   verify_current_thread();
   verify_writable_buffer_not_readlocked();

   SRenderBuffer &buffer = GetWritableRenderBuffer();

   int size = buffer.gpRenderBufferCurrent - buffer.gpRenderBufferStart;
   if (size > buffer.mPeakSize)
   {
      strcpy(buffer.mPeakArea, RenderBackend()->GetCurrentAreaDebugName());
      buffer.mPeakSize = size;
   }

   buffer.gpRenderBufferCurrent = buffer.gpRenderBufferStart;
   buffer.gpRenderCommandHead = NULL;
   buffer.gpRenderCommandTail = NULL;
}

//----------------------------------------------------------------------------

char* BP_RB_Alloc(int size)
{
   char* address = BP_RB_GetCurrentPtr();

   BP_RB_SetCurrentPtr(address + size);
   return address;
}

//----------------------------------------------------------------------------

char* BP_RB_GetCurrentPtr()
{
   char* result = GetWritableRenderBuffer().gpRenderBufferCurrent;

   // Make address 16 byte aligned
   result = (char*)(((unsigned int)result + 15) & (~15));

   return result;
}

//----------------------------------------------------------------------------

void BP_RB_SetCurrentPtr(char* ptr)
{
   BPE_ASSERT(ptr <= GetWritableRenderBuffer().gpRenderBufferEnd, "Ptr outside of buffer range.");

   GetWritableRenderBuffer().gpRenderBufferCurrent = ptr;
}

//----------------------------------------------------------------------------

#ifndef GOLD_VERSION

BPE_NOINLINE
static int _bp_rb_logcommand( unsigned int id, void *data )
{
   switch ( id )
   {
   case kCmd_Label:
      {
         SBP_RenderLabel const *pLabel = (SBP_RenderLabel const *) data;

         printf( "RB Debug: id:LABEL name:%s flags:%8.8x\n", 
            pLabel->string,
            pLabel->flags );
      }
      break;
   case kCmd_DmaPack_AutoPacket:
   case kCmd_DmaPack_Direct:
      {
         SBP_RenderDmaPack const *pDmaPack = (SBP_RenderDmaPack const *) data;
         DG_DMAPACK const *gameDmaPack = (DG_DMAPACK const *) pDmaPack->dmapack;
         printf( "RB Debug: id:%s fname:%s\n", ( id==kCmd_DmaPack_AutoPacket )? "DMAAUTO" : "DMADIRECT", gameDmaPack->fname );
      }
      break;
   default:
      if ( id == sLogLastId )
      {
         ++sLogRepeatCount;
      }
      else
      {
         sLogRepeatCount = 0;
      }
      printf( "RB Debug: id: %8.8x data: %8.8x r:%d\n", id, data, sLogRepeatCount );
      break;
   }

   sLogLastId = id;
   return 1;
}

#endif

//----------------------------------------------------------------------------

void BP_RB_AddCommand(unsigned int id, void* data)
{
   SRenderCommand* pCmd = (SRenderCommand*)BP_RB_Alloc(sizeof(SRenderCommand));
   
#ifndef GOLD_VERSION
   if ( sLogDepth )
   {
      if ( !_bp_rb_logcommand( id, data ) )
      {
         return;
      }
   }
#endif

   pCmd->mId = id;
   pCmd->mData = reinterpret_cast<char *>( data );
   pCmd->mpNext = NULL;

   SRenderBuffer &buffer = GetWritableRenderBuffer();

   if( !buffer.gpRenderCommandHead )
      buffer.gpRenderCommandHead = pCmd;

   if( buffer.gpRenderCommandTail )
      buffer.gpRenderCommandTail->mpNext = pCmd;

   buffer.gpRenderCommandTail = pCmd;
}

//----------------------------------------------------------------------------

SRenderCommand* BP_RB_GetFirstCommand(int const bufferIndex)
{
   return gRenderBuffers[bufferIndex].gpRenderCommandHead;
}

//----------------------------------------------------------------------------

int BP_RB_IsCommandValid( int const bufferIndex, SRenderCommand const *cmd )
{
   return cmd >= gRenderBuffers[ bufferIndex ].gpRenderCommandHead && cmd <= gRenderBuffers[ bufferIndex ].gpRenderCommandTail;
}

//----------------------------------------------------------------------------

void BP_RB_CopyTexture(void** ppDG_TEX_BP_, void* pDG_TEX_)
{
   DG_TEX_BP** ppDG_TEX_BP = (DG_TEX_BP**)ppDG_TEX_BP_;
   DG_TEX* pDG_TEX = (DG_TEX*)pDG_TEX_;
   if( pDG_TEX )
   {
      *ppDG_TEX_BP = (DG_TEX_BP*)BP_RB_Alloc(sizeof(DG_TEX_BP));
      BP_CopyDGTexToDGTexBP(*ppDG_TEX_BP, pDG_TEX);
   }
   else
   {
      *ppDG_TEX_BP = NULL;
   }
}

//----------------------------------------------------------------------------

#if BP_ENABLE_PROFILE_MARKERS
void BP_RB_PushMarker(char* pString)
{
   int const stringSize = strlen(pString) + 1;
   char* pStringStorage = BP_RB_Alloc(stringSize);
   memcpy(pStringStorage, pString, stringSize);
   BP_RB_AddCommand(kCmd_PushProfileMarker, pStringStorage);
//   BP_Debug_PushCPUMarker( pString );
}
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_PROFILE_MARKERS
void BP_RB_PopMarker()
{
   BP_RB_AddCommand(kCmd_PopProfileMarker, NULL);
//   BP_Debug_PopCPUMarker();
}
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_PROFILE_MARKERS
void BP_RB_PushRegionMarker(unsigned int color, char* pString)
{
   int const stringSize = strlen(pString) + 1;

   SBP_ProfileRegionMarker* pData = (SBP_ProfileRegionMarker*)BP_RB_Alloc(sizeof(SBP_ProfileRegionMarker) + stringSize);

   pData->color = color;
   memcpy(pData->string, pString, stringSize);

   BP_RB_AddCommand(kCmd_PushProfileRegionMarker, (char*)pData);
}
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_PROFILE_MARKERS
void BP_RB_PopRegionMarker()
{
   BP_RB_AddCommand(kCmd_PopProfileRegionMarker, NULL);
}
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_RENDER_LABELS
void BP_RB_AddLabel(char* pString, unsigned int flags)
{
   int const stringSize = strlen(pString) + 1;

   SBP_RenderLabel* pData = (SBP_RenderLabel*)BP_RB_Alloc(sizeof(SBP_RenderLabel) + stringSize);

   pData->flags = flags;
   memcpy(pData->string, pString, stringSize);

   BP_RB_AddCommand(kCmd_Label, (char*)pData);
}
#endif

//----------------------------------------------------------------------------

void BP_RB_DebugBeginLog( char const *name )
{
   ++sLogDepth;

   printf( "*** RB Begin Log: %s (depth:%d)\n", name, sLogDepth );

   sLogLastId = 0xFFFFFFFF;
   sLogRepeatCount = 0;
}

//----------------------------------------------------------------------------

void BP_RB_DebugEndLog()
{
   printf( "*** RB End Log (depth:%d)\n", sLogDepth );
   --sLogDepth;
   sLogLastId = 0xFFFFFFFF;
   sLogRepeatCount = 0;
}

};
