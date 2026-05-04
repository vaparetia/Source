//----------------------------------------------------------------------------
// CArchiveDecompressionSupport.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "stdafx.h"

#include "CArchiveDecompressionSupport.h"

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void CArchiveDecompressionSupport::Initialize( const unsigned int decompressedBlockSize, const unsigned int taskQueueSize )
{
   mDecompressionBuffer.resize( decompressedBlockSize, 0 );

   //Initialize decompression context.
   HRESULT hr;
   XMEMCODEC_PARAMETERS_LZX codecParams = 
   {
      0,  //flags
      128*1024, //window size
      0  //compression partition size (ignored for decompression)
   };
   memset( &mDecompressionContext, 0, sizeof( mDecompressionContext ) );
   hr = XMemCreateDecompressionContext( XMEMCODEC_LZX, &codecParams, 0, &mDecompressionContext );
   BPE_ASSERT_NO_MSG( !FAILED( hr ) );

   // Create a lock-free queue that blocks on Remove().
   XLOCKFREE_CREATE xlfinfo = { 0 };
   xlfinfo.attributes = XLOCKFREE_REMOVE_WAIT;
   xlfinfo.removeWaitTime = INFINITE;
   xlfinfo.maximumLength = 0;

   mTaskQueue.Initialize( &xlfinfo );

   //Start up worker thread.
   mThreadHandle = CreateThread( NULL, 0, &StaticThreadEntry, this, CREATE_SUSPENDED, NULL );
   assert( mThreadHandle );   // Tell code analysis to assume that CreateThread won't fail.
   XSetThreadProcessor( mThreadHandle, 4 );
   ResumeThread( mThreadHandle );
}

void CArchiveDecompressionSupport::Shutdown()
{
   //Need to shut down worker thread
   BPE_ASSERT( false, "TODO!" );

   XMemDestroyDecompressionContext( mDecompressionContext );
}

void CArchiveDecompressionSupport::AddDecompressionJob( SArchiveDecompressionJob * pJob )
{
   BPE_ASSERT( mDecompressionBuffer.size() != 0, "Not initialized!" );
   HRESULT hr = mTaskQueue.Add( pJob );
   BPE_ASSERT( hr != XLOCKFREE_STRUCTURE_FULL, "Failed to add decompression job" );
}

DWORD CArchiveDecompressionSupport::StaticThreadEntry( LPVOID pWorkerThreadObject )
{
   ( ( CArchiveDecompressionSupport* )pWorkerThreadObject )->WorkerThreadFn();
   return 0;
}

void CArchiveDecompressionSupport::WorkerThreadFn()
{
   while( SArchiveDecompressionJob * pJob = mTaskQueue.Remove() )
   {
      // Execute the work function.
      DoDecompressionJob( pJob );
   }
}

void CArchiveDecompressionSupport::DoDecompressionJob( const SArchiveDecompressionJob * const pJob )
{
   BPE_ASSERT_NO_MSG( pJob->mDecompressedBlockSize <= mDecompressionBuffer.size() );
   BPE_ASSERT_NO_MSG( pJob->mSrcBlockSize <= mDecompressionBuffer.size() );
   BPE_ASSERT_NO_MSG( pJob->mDecompressedOffsetWithinBlock < pJob->mDecompressedBlockSize );
   BPE_ASSERT_NO_MSG( pJob->mDecompressedSizeWithinBlock <= pJob->mDecompressedBlockSize );

   unsigned char * const pDecompressionBuffer = &mDecompressionBuffer[0];

   const bool bCompressedBlock = ( pJob->mSrcBlockSize < pJob->mDecompressedBlockSize );

   if( bCompressedBlock )
   {
      //Decompress from this buffer.
      HRESULT hr;
      SIZE_T decompressedBlockSize = mDecompressionBuffer.size();
      hr = XMemDecompress( mDecompressionContext, pDecompressionBuffer, &decompressedBlockSize, pJob->mSrcBlockData, pJob->mSrcBlockSize );
      BPE_ASSERT_NO_MSG( !FAILED( hr ) );
   }
   else
   {
      //copy uncompressed data straight out.
      memcpy( pDecompressionBuffer, pJob->mSrcBlockData, pJob->mSrcBlockSize );
   }

   //Copy out what we want from the final de/uncompressed block.
   memcpy( pJob->mpDst, pDecompressionBuffer + pJob->mDecompressedOffsetWithinBlock, pJob->mDecompressedSizeWithinBlock );

   if( pJob->mCallbackFn )
   {
      pJob->mCallbackFn( pJob );
   }

   //TODO: can use a ring buffer to allocate / free compressed block data.
   free( (void*)pJob );
}
