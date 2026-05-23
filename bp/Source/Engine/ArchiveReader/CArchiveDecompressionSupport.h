//----------------------------------------------------------------------------
// CArchiveDecompressionSupport.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

#if BPE_TARGET == BPE_TARGET_X360

#include <xtl.h>
#include <xmcore.h>

//----------------------------------------------------------------------------

struct SArchiveDecompressionJob;

typedef void (*TArchiveDecompressionCallbackFn)( const SArchiveDecompressionJob * const pJob );

#pragma warning( disable:4200 ) //warning C4200: nonstandard extension used : zero-sized array in struct/union
struct SArchiveDecompressionJob
{
   TArchiveDecompressionCallbackFn  mCallbackFn;
   void *                           mUserParm;
   void *                           mpDst;
   unsigned int                     mDecompressedBlockSize;
   unsigned int                     mDecompressedOffsetWithinBlock;
   unsigned int                     mDecompressedSizeWithinBlock;
   unsigned int                     mSrcBlockSize;
   unsigned char                    mSrcBlockData[0];
};
#pragma warning( default:4200 )

class CArchiveDecompressionSupport
{
public:
   void Initialize( const unsigned int decompressedBlockSize, const unsigned int taskQueueSize );
   void Shutdown();

   void AddDecompressionJob( SArchiveDecompressionJob * pJob );

private:

   static DWORD StaticThreadEntry( LPVOID pWorkerThreadObject );
   void WorkerThreadFn();

   void DoDecompressionJob( const SArchiveDecompressionJob * const pJob );

private:
   XLockFreeQueue <SArchiveDecompressionJob> mTaskQueue;
   HANDLE                                    mThreadHandle;

   std::vector< unsigned char >              mDecompressionBuffer;

   XMEMDECOMPRESSION_CONTEXT                 mDecompressionContext;
};

//----------------------------------------------------------------------------

#endif //BPE_TARGET_X360