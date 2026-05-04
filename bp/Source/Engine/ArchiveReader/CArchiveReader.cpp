//----------------------------------------------------------------------------
// CArchiver.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include "CArchiveReader.h"
#include "CArchiveDecompressionSupport.h"

#include "Engine/Mechanics/HashUtils.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/System/CStopWatch.h"

#pragma warning(disable:4351) // warning C4351: new behavior: elements of array 'SArchiveRamCacheBlock::mFilename' will be default initialized

//#define BP_ARCHIVER_LOAD_NEWER_FROM_FS //BP - Comment in to load files from file system rather than archive when newer

//----------------------------------------------------------------------------

namespace
{
   const unsigned int   skDvdSectorSize = 2048;

   const unsigned int   skMagicNumber = 'XBAR';
   const unsigned int   skVersionNumber = 0;

   const unsigned int   skRamCacheBlockCount = 4;
   const unsigned int   skRamCacheBlockSize = 512 * 1024;

   const unsigned int   skFileHandleCount = 16;
   const unsigned int   skFileOpCount = 16;

   const unsigned int   skBlockSize = 128 * 1024;

   bool                 sbDebugPrintRamCache = false;

   const unsigned int   skMaxDecompressionJobCount = 128;
   CArchiveDecompressionSupport  sDecompressionSupport;
}

//----------------------------------------------------------------------------

extern "C" void BP_UnifyPathX360( char * path )
{
#if BPE_TARGET == BPE_TARGET_X360
   int totalLen = 0;
   while( path[totalLen] != NULL )
   {
      if( path[totalLen] == '/' )
      {
         path[totalLen] = '\\';
      }
      ++totalLen;
   }
#endif
}

//----------------------------------------------------------------------------

SArchiveData::SArchiveData()
: mFileHandle(INVALID_HANDLE_VALUE)
, mArchiveSize( 0 )
, mArchiveFileModificationDateTime( 0 )
, mIsCommonArchive(false)
, mIsInitCommonArchive(false)
, mArchiveInit_ThreadHandle(NULL)
{
   memset( &mHeader, 0, sizeof( mHeader ) );
}

SArchiveData::~SArchiveData()
{
   CloseHandle(mFileHandle);
   mFileHandle = INVALID_HANDLE_VALUE;
   mTOC.clear();
}

DWORD __stdcall InitializeCommonArchive_Thread( LPVOID InThis )
{
   SArchiveData * pAD = (SArchiveData *)InThis;
   pAD->InitializeCommonArchive_Internal();

   return 0;
}

void SArchiveData::InitializeCommonArchive_Internal()
{
   // We need to get the entire file into memory
   mRamByteData.resize( (unsigned int)mArchiveSize );
   int numBytesRead;
   ::SetFilePointer(mFileHandle, 0, 0, FILE_BEGIN);
   ::ReadFile(mFileHandle, &mRamByteData[0], (unsigned int)mArchiveSize, (LPDWORD)&numBytesRead, NULL );
}

void SArchiveData::StartCommonArchiveInitializeThread()
{
   // Don't start initialization of common archive if it's either already initialized or in progress of being initialized.
   if( mIsInitCommonArchive || mArchiveInit_ThreadHandle )
      return;

   mArchiveInit_ThreadHandle = CreateThread(NULL, 0, InitializeCommonArchive_Thread, this, CREATE_SUSPENDED, &mArchiveInit_ThreadD);
   XSetThreadProcessor(mArchiveInit_ThreadHandle, 5);
   ResumeThread(mArchiveInit_ThreadHandle);
}

void SArchiveData::Initialize( const char * const path )
{
   mFilePath = path;

   mFileHandle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   BPE_ASSERT( mFileHandle != INVALID_HANDLE_VALUE, "CArchiveReader should check for existence" );

   mArchiveSize = GetFileSize( mFileHandle, NULL );
   FILETIME filetime;
   GetFileTime( mFileHandle, NULL, NULL, &filetime );
   mArchiveFileModificationDateTime = ((ULARGE_INTEGER*)(&filetime))->QuadPart;

   if( mIsCommonArchive )
   {
      //initialize TOC structure
      int numBytesRead;
      ::ReadFile(mFileHandle, &mHeader, sizeof(mHeader), (LPDWORD)&numBytesRead, NULL);
      BPE_ASSERT( mHeader.mMagicNumber == skMagicNumber && mHeader.mVersionNumber == skVersionNumber, "Bad header" );
      mTOC.resize( mHeader.mFileCount );
      ::ReadFile(mFileHandle, &mTOC[0], mTOC.size() * sizeof( mTOC[0] ), (LPDWORD)&numBytesRead, NULL);
      mCompressedBlockSizes.resize( mHeader.mBlockCount );
      ::ReadFile(mFileHandle, &mCompressedBlockSizes[0], mCompressedBlockSizes.size() * sizeof(mCompressedBlockSizes[0]), (LPDWORD)&numBytesRead, NULL );
      // We will init the common archive when requested or when we find the first file needed from the common archive
   }
   else
   {
      //initialize TOC structure
      int numBytesRead;
      ::ReadFile(mFileHandle, &mHeader, sizeof(mHeader), (LPDWORD)&numBytesRead, NULL);
      BPE_ASSERT( mHeader.mMagicNumber == skMagicNumber && mHeader.mVersionNumber == skVersionNumber, "Bad header" );
      mTOC.resize( mHeader.mFileCount );
      ::ReadFile(mFileHandle, &mTOC[0], mTOC.size() * sizeof( mTOC[0] ), (LPDWORD)&numBytesRead, NULL);
      mCompressedBlockSizes.resize( mHeader.mBlockCount );
      ::ReadFile(mFileHandle, &mCompressedBlockSizes[0], mCompressedBlockSizes.size() * sizeof(mCompressedBlockSizes[0]), (LPDWORD)&numBytesRead, NULL );
   }
}

//----------------------------------------------------------------------------

SArchiveRamCacheBlock::SArchiveRamCacheBlock()
: mFilename()
, mOffset( 0 )
, mData( new unsigned char[skRamCacheBlockSize] )
, mSize( 0 )
, mLastTouchedCount( 0 )
, mFileHandle(INVALID_HANDLE_VALUE)
, mOverlapped()
, mFilledBytes()
, mLockCount( 0 )
{
   memset( mFilename, 0, sizeof( mFilename ) );
}

//----------------------------------------------------------------------------

CArchiveReader::CArchiveReader()
: mArchives()
{
   mRamCache.resize( skRamCacheBlockCount );
   mRamCacheTouchCount = 0;

   mArchives.reserve( 2 ); //worst case, we should need one per layer.
}

bool CArchiveReader::HasArc() const 
{ 
   return mArchives.size() > 0;
}

void CArchiveReader::Initialize()
{
   sDecompressionSupport.Initialize( skBlockSize, skMaxDecompressionJobCount );

   mFileHandlePool.reserve( skFileHandleCount );
   for( int i=0; i < skFileHandleCount; ++i )
   {
      mFileHandlePool.push_back( new CArchiveFileHandle() );
   }
   mFileOpPool.reserve( skFileOpCount );
   for( int i=0; i < skFileOpCount; ++i )
   {
      mFileOpPool.push_back( new CArchiveOp( skBlockSize ) );
   }
}

void CArchiveReader::StartCommonArchiveInitializeThread()
{
   for( TArchiveDataVector::iterator archiveAt = mCommonArchives.begin(); archiveAt != mCommonArchives.end(); ++archiveAt )
   {
      SArchiveData & archive = *archiveAt;
      archive.StartCommonArchiveInitializeThread();
   }
}

bool CArchiveReader::OpenArchive(const char * const path, bool isCommonArchive)
{
   mArchiveFolderPath = path;
   int lastSlash = mArchiveFolderPath.find_last_of("/\\");
   mArchiveFolderPath = mArchiveFolderPath.substr( 0, lastSlash );

   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesEx( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      return false;
   }

   SArchiveData* pArchive = NULL;
   if( !isCommonArchive )
   {
      mArchives.push_back( SArchiveData() );
      pArchive = &mArchives.back();
   }
   else
   {
      mCommonArchives.push_back( SArchiveData() );
      pArchive = &mCommonArchives.back();
   }
   pArchive->mIsCommonArchive = isCommonArchive;
   pArchive->Initialize( path );

   BPE_ASSERT( skBlockSize == pArchive->mHeader.mBlockSize, "all archives must have same size blocks!" );

   return true;
}

int CArchiveReader::Shutdown()
{
   if( HasArc() )
   {
      mArchives.clear();
   }

   sDecompressionSupport.Shutdown();

   return 1;
}

bool CArchiveReader::FileExists( const char * const path, const char * const commonPath ) const
{
   const SArchiveTOCEntry * pEntry = FindTOCEntry( path, commonPath, NULL );
   if( pEntry != NULL )
      return true;

   //Look for this file in the loose file system.
   char buffer[MAX_PATH];
   strcpy(buffer, mArchiveFolderPath.c_str() );
   strcat( buffer, "/" );
   strcat( buffer, path );
   BP_UnifyPathX360(buffer);

   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesEx( buffer, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      return false;
   }

   return true;
}

long CArchiveReader::GetFileSizeAttr( const char * const path, const char * const commonPath ) const
{
   const SArchiveData * pArchive = NULL;
   const SArchiveTOCEntry * pTOCEntry = FindTOCEntry( path, commonPath, &pArchive );
   unsigned __int64 fileSize;

#ifdef BP_ARCHIVER_LOAD_NEWER_FROM_FS
   bool bPreferLooseFile = true;
#else
   bool bPreferLooseFile = (pTOCEntry == NULL);
#endif

   if( pTOCEntry != NULL )
   {
      //Take file size from the archive.
      fileSize = pTOCEntry->mOriginalSize;
   }

   if( bPreferLooseFile )
   {
      //Look for this file in the loose file system.
      char buffer[MAX_PATH];
      strcpy(buffer, mArchiveFolderPath.c_str() );
      strcat( buffer, "/" );
      strcat( buffer, path );
      BP_UnifyPathX360(buffer);

      WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
      BOOL b = GetFileAttributesEx( buffer, GetFileExInfoStandard, &fileAttr );
      if( b )
      {
         //Loose file exists.
#ifdef BP_ARCHIVER_LOAD_NEWER_FROM_FS
         //Take the loose file only if it's newer than the archive.
         unsigned __int64 fileModificationDateTime = ((ULARGE_INTEGER*)(&fileAttr.ftLastWriteTime))->QuadPart;
         if( pArchive == NULL || fileModificationDateTime > pArchive->mArchiveFileModificationDateTime )
#endif
         {
            fileSize = ((unsigned __int64)(fileAttr.nFileSizeHigh) << 32 ) | fileAttr.nFileSizeLow;
         }
      }
      else
      {
         //Loose file does not exist.  This case is only valid if the file is in the archive.
         BPE_ASSERT( pTOCEntry != NULL, "File not found!" );
      }
   }

   return (long)fileSize;
}

const SArchiveTOCEntry * CArchiveReader::FindTOCEntry( const char * const path, const char * const commonPath, const SArchiveData ** const ppOutArchive ) const
{
   const SArchiveTOCEntry * pTOCEntry = NULL;
   const SArchiveData * pArchive = NULL;
   if( commonPath )
   {
      for( TArchiveDataVector::const_iterator archiveAt = mCommonArchives.begin(); archiveAt != mCommonArchives.end(); ++archiveAt )
      {
         const SArchiveData & archive = *archiveAt;
         if( pTOCEntry = FindTOCEntry( archive, commonPath ) )
         {
            pArchive = &archive;
            break;
         }
      }
   }
   if( pTOCEntry == NULL )
   {
      for( TArchiveDataVector::const_iterator archiveAt = mArchives.begin(); archiveAt != mArchives.end(); ++archiveAt )
      {
         const SArchiveData & archive = *archiveAt;
         if( pTOCEntry = FindTOCEntry( archive, path ) )
         {
            pArchive = &archive;
            break;
         }
      }
   }

   if( ppOutArchive )
   {
      *ppOutArchive = pArchive;
   }
   return pTOCEntry;
}

const SArchiveTOCEntry * CArchiveReader::FindTOCEntry( const SArchiveData & archive, const char * const path ) const
{
   //try to find path
   std::string fullPath = path;
   if( !archive.mIsCommonArchive )
      fullPath.insert(fullPath.begin(),'/');
   unsigned char sha1Hash[20];
   {
      XHASH_STATE_SHA1 hashState;
#pragma warning(disable:4995)
      XHashMemoryBeginSHA1Deprecated(&hashState);
      XHashMemoryUpdateSHA1Deprecated(&hashState,fullPath.c_str(),fullPath.size());
      XHashMemoryEndSHA1Deprecated(&hashState,(BYTE*)sha1Hash);
#pragma warning(default:4995)
   }

   SArchiveData::TTOC::const_iterator it = std::lower_bound( archive.mTOC.begin(), archive.mTOC.end(), sha1Hash );
   if( it != archive.mTOC.end() && ( memcmp( it->mArchivePathHash, sha1Hash, sizeof(sha1Hash)) == 0 ) )
   {
      const SArchiveTOCEntry * const pTOCEntry = &*it;
      return pTOCEntry;
   }

   //File not in this archive.
   return NULL;
}

const char * CArchiveReader::GetPhysicalFileName( const CArchiveFileHandle * const pFileHandle ) const
{
   if( pFileHandle->mpArchive )
   {
      return pFileHandle->mpArchive->mFilePath.c_str();
   }
   else
   {
      return pFileHandle->mFilename;
   }
}

CArchiveFileHandle * CArchiveReader::FindFreeFileHandle()
{
   for( TFileHandleVector::iterator it = mFileHandlePool.begin(); it != mFileHandlePool.end(); ++it )
   {
      CArchiveFileHandle * const pFileHandle = *it;
      if( !pFileHandle->mbInUse )
      {
         pFileHandle->mbInUse = true;
         return pFileHandle;
      }
   }

   BPE_ASSERT( false, "Increase size of file handle pool" );
   return NULL;
}

CArchiveOp * CArchiveReader::FindFreeFileOp()
{
   for( TFileOpVector::iterator it = mFileOpPool.begin(); it != mFileOpPool.end(); ++it )
   {
      CArchiveOp * const pFileOp = *it;
      if( !pFileOp->mbInUse )
      {
         pFileOp->mbInUse = true;
         return pFileOp;
      }
   }

   BPE_ASSERT( false, "Increase size of file op pool" );
   return NULL;
}

CArchiveFileHandle * CArchiveReader::OpenFile( const char * const path, const char * const commonPath )
{
   const SArchiveData * pArchive = NULL;
   const SArchiveTOCEntry * pTOCEntry = NULL;
   char fullPath[FILENAME_MAX];
   unsigned __int64 fileSize;
   
   for( TArchiveDataVector::const_iterator archiveAt = mArchives.begin(); archiveAt != mArchives.end(); ++archiveAt )
   {
      const SArchiveData & archive = *archiveAt;
      if( pTOCEntry = FindTOCEntry( archive, path ) )
      {
         pArchive = &archive;
         break;
      }
   }
   if( pArchive == NULL && commonPath )
   {
      for( TArchiveDataVector::const_iterator archiveAt = mCommonArchives.begin(); archiveAt != mCommonArchives.end(); ++archiveAt )
      {
         const SArchiveData & archive = *archiveAt;
         if( pTOCEntry = FindTOCEntry( archive, commonPath ) )
         {
            pArchive = &archive;
            break;
         }
      }
   }

#ifdef BP_ARCHIVER_LOAD_NEWER_FROM_FS
   bool bPreferLooseFile = true;
#else
   bool bPreferLooseFile = (pTOCEntry == NULL);
#endif

   if( pTOCEntry )
   {
      //Take data from the archive.
      BPE_ASSERT_NO_MSG( pTOCEntry->mBlockListStartIndex < pArchive->mCompressedBlockSizes.size() );
      strcpy( fullPath, path );
      fileSize = pTOCEntry->mOriginalSize;
   }

   if( bPreferLooseFile )
   {
      //Search for loose file in file system if it's not in the archive.
      std::string looseFilePath = mArchiveFolderPath + "/" + path;
      strcpy( fullPath, mArchiveFolderPath.c_str() );
      strcat( fullPath, "/" );
      strcat( fullPath, path );
      BP_UnifyPathX360(fullPath);

      WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
      BOOL b = GetFileAttributesEx( fullPath, GetFileExInfoStandard, &fileAttr );
      if( b )
      {
         //Loose file exists.
#ifdef BP_ARCHIVER_LOAD_NEWER_FROM_FS
         //Take the loose file only if it's newer than the archive.
         unsigned __int64 fileModificationDateTime = ((ULARGE_INTEGER*)(&fileAttr.ftLastWriteTime))->QuadPart;
         if( pArchive == NULL || fileModificationDateTime > pArchive->mArchiveFileModificationDateTime )
         {
            fileSize = ((unsigned __int64)(fileAttr.nFileSizeHigh) << 32 ) | fileAttr.nFileSizeLow;
            pTOCEntry = NULL;
            pArchive = NULL;
         }
         else
         {
            //Revert the fullPath to the one corresponding to the TOC entry.
            strcpy( fullPath, path );
         }
#else
         //If we got here without BP_ARCHIVER_LOAD_NEWER_FROM_FS, then we must take the loose file because
         //the file is not in the archive.
         fileSize = ((unsigned __int64)(fileAttr.nFileSizeHigh) << 32 ) | fileAttr.nFileSizeLow;
         BPE_ASSERT( pArchive == NULL && pTOCEntry == NULL, "If TOC / Archive was NULL, should not have gotten here!" );
#endif
      }
      else
      {
         //Loose file does not exist.  This case is only valid if the file is in the archive.
         BPE_ASSERT( pTOCEntry != NULL, "File not found!" );
      }
   }

   CArchiveFileHandle * pFileHandle = FindFreeFileHandle();
   pFileHandle->mpArchive = pArchive;
   pFileHandle->mpTOCEntry = pTOCEntry;
   strcpy( pFileHandle->mFilename, fullPath );
   pFileHandle->mVirtualSize = fileSize;

   return pFileHandle;
}

void CArchiveReader::CloseFile( CArchiveFileHandle* pFileHandle )
{
   DeleteOp( pFileHandle->mpOp );
   pFileHandle->Reset();
}

int CArchiveReader::ReadFile(CArchiveFileHandle * pFileHandle, void * pBuf, unsigned int byteCount)
{
   CArchiveOp * pOp = ReadFileAsync( pFileHandle, pBuf, byteCount );
   while( !IsOpComplete( pOp ) )
   {
      Sleep(1);
   }
   DeleteOp( pOp );
   return 0;
}

CArchiveOp * CArchiveReader::ReadFileAsync(CArchiveFileHandle * pFileHandle, void * pBuf, unsigned int byteCount)
{
   BPE_ASSERT_NO_MSG( pFileHandle != NULL );
   BPE_ASSERT_NO_MSG( pFileHandle->mpOp == NULL ); //already an async operation going!

   unsigned int readBytes = bpe::min_val( byteCount, (unsigned int)(pFileHandle->GetFileSize() - pFileHandle->mVirtualCurrOffset) );

   CArchiveOp * pOp = FindFreeFileOp();
   pOp->mpFileHandle = pFileHandle;
   pOp->mOffset = pFileHandle->mVirtualCurrOffset;
   pOp->mpBuf = pBuf;
   pOp->mByteCount = readBytes;

   pFileHandle->mpOp = pOp;

   if( pFileHandle->mpTOCEntry )
   {
      const SArchiveData * const pArchive = pFileHandle->mpArchive;
      //Load initial block index and offset here.
      const unsigned int * const blockSizeList = &pArchive->mCompressedBlockSizes[pFileHandle->mpTOCEntry->mBlockListStartIndex];
      int currVirtualBlockOffset = 0;
      for( pOp->mCurrBlockIndex = 0, pOp->mCurrBlockOffset = 0;; )
      {
         //virtualBlockSize normally the full uncompressed block size except when it's the last block of a file, when it might be smaller.
         unsigned int virtualBlockSize = bpe::min_val( pArchive->mHeader.mBlockSize, pFileHandle->mpTOCEntry->mOriginalSize - currVirtualBlockOffset );
         if( currVirtualBlockOffset + virtualBlockSize > pOp->mOffset )
         {
            //next one will be the first one past the block where we want to start.
            break;
         }

         unsigned int blockSize = blockSizeList[pOp->mCurrBlockIndex];
         BPE_ASSERT( blockSize < pArchive->mHeader.mBlockSize, "inconsistent block size!" );
         if( blockSize == 0 )
         {
            //This block is uncompressed.  Block size is the same as the virtual block size.
            blockSize = virtualBlockSize;
         }

         ++pOp->mCurrBlockIndex;
         pOp->mCurrBlockOffset += blockSize;
         currVirtualBlockOffset += pArchive->mHeader.mBlockSize;   //update virtual by a full uncompressed block size.
      }
   }

   UpdateOp( pOp );

   return pOp;
}

CArchiveOp * CArchiveReader::ReadFileAsync( const char * const path, void * pBuf, const unsigned int offset, unsigned int byteCount )
{
   CArchiveFileHandle * fp = OpenFile( path, NULL );
   fp->Seek( offset, SEEK_SET );
   CArchiveOp * pOp = ReadFileAsync( fp, pBuf, byteCount );
   pOp->mbFileOwner = true;
   return pOp;
}

bool CArchiveReader::IsOpComplete( CArchiveOp * pOp )
{
   UpdateOp( pOp );

   BPE_ASSERT_NO_MSG( pOp->mWrittenBytes <= pOp->mByteCount );   //internal error; read too far!
   return pOp->mWrittenBytes == pOp->mByteCount;
}

void CArchiveReader::DeleteOp( CArchiveOp * pOp )
{
   if( !pOp )
   {
      return;
   }

   BPE_ASSERT_NO_MSG( pOp->mpFileHandle->mpOp == pOp );

   while( !IsOpComplete( pOp ) )
   {
      Sleep( 1 );
   }

   pOp->mpFileHandle->mpOp = NULL;

   if( pOp->mbFileOwner )
   {
      CloseFile( pOp->mpFileHandle );
   }

   pOp->Reset();
}

void CArchiveReader::UpdateOp( CArchiveOp * const pOp )
{
   CArchiveFileHandle * pFileHandle = pOp->mpFileHandle;

   if( pFileHandle->mpTOCEntry )
   {
      //Reading from the archive.
      UpdateReadArchive( pOp );
   }
   else
   {
      //loose file.
      UpdateReadLoose( pOp );
   }
}

void CArchiveReader::UpdateReadArchive( CArchiveOp * const pOp )
{
   CArchiveFileHandle * const pFileHandle = pOp->mpFileHandle;
   const SArchiveData * const pArchive = pFileHandle->mpArchive;
   const unsigned int * const blockSizeList = &pArchive->mCompressedBlockSizes[pFileHandle->mpTOCEntry->mBlockListStartIndex];
   unsigned int currVirtualBlockOffset = pOp->mCurrBlockIndex * pOp->mBlockBuffer.size(); //virtual offset of current block within original uncompressed file
   while( pOp->mVirtualReadBytes < pOp->mByteCount )
   {
      //still reading data.

      //virtualBlockSize normally the full uncompressed block size except when it's the last block of a file, when it might be smaller.
      unsigned int virtualBlockSize = bpe::min_val( pArchive->mHeader.mBlockSize, pFileHandle->mpTOCEntry->mOriginalSize - currVirtualBlockOffset );

      unsigned int blockSize = blockSizeList[pOp->mCurrBlockIndex];
      BPE_ASSERT( blockSize < pArchive->mHeader.mBlockSize, "inconsistent block size!" );
      bool bCompressedBlock = true;
      if( blockSize == 0 )
      {
         //This block is uncompressed.  Block size is the same as the virtual block size.
         blockSize = virtualBlockSize;
         bCompressedBlock = false;
      }

      unsigned char * const pBlockCache = &pOp->mBlockBuffer[0];
      if( const int readBlockBytes = TryReadRamCache( pOp, pFileHandle->mpTOCEntry->mStartOffset + pOp->mCurrBlockOffset + pOp->mBlockBufferFilledBytes, pBlockCache + pOp->mBlockBufferFilledBytes, blockSize - pOp->mBlockBufferFilledBytes ) )
      {
         pOp->mBlockBufferFilledBytes += readBlockBytes;
         BPE_ASSERT( pOp->mBlockBufferFilledBytes <= blockSize, "block cache overflow???" );

         if( pOp->mBlockBufferFilledBytes == blockSize )
         {
            //We have an entire block of data ready for decompression (or straight copying if not compressed).
            void * const pDst = (unsigned char*)pOp->mpBuf + pOp->mVirtualReadBytes;
            const unsigned int remain = pOp->mByteCount - pOp->mVirtualReadBytes;

            unsigned int virtualOffsetWithinBlock = (unsigned int)(pOp->mOffset + pOp->mVirtualReadBytes - currVirtualBlockOffset);
            unsigned int virtualSizeWithinBlock = virtualBlockSize - virtualOffsetWithinBlock;

            unsigned int writeSize = bpe::min_val( remain, virtualSizeWithinBlock );

            //TODO: can get rid of this allocation by using a ring buffer in the decompression support.
            SArchiveDecompressionJob * const pJob = (SArchiveDecompressionJob *)malloc( sizeof( SArchiveDecompressionJob ) + blockSize );
            pJob->mCallbackFn = CArchiveOp::DecompressionJobCompleteCallback;
            pJob->mUserParm = (void*)pOp;
            pJob->mpDst = pDst;
            pJob->mDecompressedBlockSize = virtualBlockSize;
            pJob->mDecompressedOffsetWithinBlock = virtualOffsetWithinBlock;
            pJob->mDecompressedSizeWithinBlock = writeSize;
            pJob->mSrcBlockSize = blockSize;
            memcpy( pJob->mSrcBlockData, pBlockCache, blockSize );
            sDecompressionSupport.AddDecompressionJob( pJob );

            pFileHandle->mVirtualCurrOffset += writeSize;
            pOp->mVirtualReadBytes += writeSize;

            //Reset block cache to empty.
            pOp->mBlockBufferFilledBytes = 0;
         }
         else
         {
            //this block straddles a cache block and we need another loop iteration to finish filling it.
            //*don't* allow the current block to be updated per the bottom of this loop.
            continue;
         }
      }
      else
      {
         //Still waiting for a cache block to finish loading.
         break;
      }

      ++pOp->mCurrBlockIndex;                         //next block index.
      pOp->mCurrBlockOffset += blockSize;             //update currOffset by actual (possibly compressed) block size.
      currVirtualBlockOffset += pArchive->mHeader.mBlockSize;   //update virtual by a full uncompressed block size.
   }
}

void CArchiveReader::UpdateReadLoose( CArchiveOp * const pOp )
{
   CArchiveFileHandle * const pFileHandle = pOp->mpFileHandle;
   while( pOp->mWrittenBytes < pOp->mByteCount )
   {
      const unsigned __int64 srcOffset = pOp->mOffset + pOp->mWrittenBytes;
      void * const pDst = (unsigned char*)pOp->mpBuf + pOp->mWrittenBytes;
      const unsigned int remain = pOp->mByteCount - pOp->mWrittenBytes;

      if( const int written = TryReadRamCache( pOp, srcOffset, pDst, remain ) )
      {
         pOp->mWrittenBytes += written;
         pFileHandle->mVirtualCurrOffset += written;
      }
      else
      {
         //Still waiting.
         break;
      }
   }
}

int CArchiveReader::TryReadRamCache(CArchiveOp * const pOp, const unsigned __int64 srcOffset, void * const pBuf, const unsigned int reqSize )
{
   if( pOp->mpFileHandle->mpArchive == NULL || !pOp->mpFileHandle->mpArchive->mIsCommonArchive )
   {
      const char * const physicalFilename = GetPhysicalFileName( pOp->mpFileHandle );
      const unsigned __int64 physicalSize = pOp->mpFileHandle->mpTOCEntry ? pOp->mpFileHandle->mpArchive->mArchiveSize : pOp->mpFileHandle->GetFileSize();
      if( SArchiveRamCacheBlock * const pCacheBlock = FindRamCacheBlockThatContains( physicalFilename, srcOffset ) )
      {
         //Cache hit.

         //If we weren't already waiting on this cache block, it's what we're waiting on now.
         if( !pOp->mpPendingCacheBlock )
         {
            ++pCacheBlock->mLockCount;
            pOp->mpPendingCacheBlock = pCacheBlock;
         }
         else
         {
            //We've either already waited on this cache block once, or we're the one who started it loading.
            //In any case, we should *not* be waiting on another at this point.
            BPE_ASSERT( pOp->mpPendingCacheBlock == pCacheBlock, "Should already be waiting on this cache block" );
         }

         UpdateCacheBlock( *pCacheBlock );

         if( pCacheBlock->mFilledBytes < pCacheBlock->mSize )
         {
            //This cache block is still loading.
            return 0;
         }

         //Copy out what we want from this cache block and return how much was copied.
         unsigned int offsetInBlock = (unsigned int)( srcOffset - pCacheBlock->mOffset );
         unsigned int sizeInBlock = (unsigned int)( pCacheBlock->mOffset + pCacheBlock->mFilledBytes - srcOffset );
         int writeSize = bpe::min_val( reqSize, sizeInBlock );

         memcpy( pBuf, pCacheBlock->mData + offsetInBlock, writeSize );

         //We were waiting for this cache block but now we're done with it.
         --pCacheBlock->mLockCount;
         pOp->mpPendingCacheBlock = NULL;

         return writeSize;
      }
      else
      {
         //Cache miss.  Find a free cache block that we can load into.
         if( SArchiveRamCacheBlock * const pCacheBlock = FindOldestFreeRamCacheBlock() )
         {
            BPE_ASSERT_NO_MSG( pCacheBlock->mLockCount == 0 ); //not really free?

            BeginLoadCacheBlock( *pCacheBlock, physicalFilename, srcOffset, physicalSize );
            //Make sure nobody else replaces this cache block until we're done with it.
            BPE_ASSERT_NO_MSG( pOp->mpPendingCacheBlock == NULL ); //should have hit!
            pOp->mpPendingCacheBlock = pCacheBlock;
            ++pCacheBlock->mLockCount;
         }
         //else we have to wait until a cache block gets freed up.

         return 0;   //didn't read anything.
      }
   }
   else
   {
      // common archive cache read
      if( pOp->mpFileHandle->mpArchive->mIsInitCommonArchive == true )
      {
         const char* pData = &pOp->mpFileHandle->mpArchive->mRamByteData[(unsigned int)srcOffset];
         memcpy( pBuf, pData, reqSize );
         return reqSize;
      }
      else
      {
         if( pOp->mpFileHandle->mpArchive->mArchiveInit_ThreadHandle == NULL )
         {
            // FAILSAFE to load the common archives if we're requesting to read from it before the game told us to load them.
            bpe_debugger_printf("WARNING: Failsafe automatic start of common archive initialization.");
            StartCommonArchiveInitializeThread();
         }
         else if( WaitForSingleObject(pOp->mpFileHandle->mpArchive->mArchiveInit_ThreadHandle, 0) == WAIT_OBJECT_0 )
         {
            pOp->mpFileHandle->mpArchive->mIsInitCommonArchive = true;
            XCloseHandle(pOp->mpFileHandle->mpArchive->mArchiveInit_ThreadHandle);
            pOp->mpFileHandle->mpArchive->mArchiveInit_ThreadHandle = NULL;
         }
         return 0; // we will read from this whenever InitializeCommonArchive_Thread finishes
      } 
   }
}

SArchiveRamCacheBlock * CArchiveReader::FindRamCacheBlockThatContains( const char * const physicalFilename, const unsigned __int64 offset )
{

   for( int i=0; i < mRamCache.size(); ++i )
   {
      SArchiveRamCacheBlock & cacheBlock = mRamCache[i];
      if( !strcmp( cacheBlock.mFilename, physicalFilename ) && ( offset >= cacheBlock.mOffset ) && ( offset < cacheBlock.mOffset + skRamCacheBlockSize ) )
      {
         //hit!
         //reset touch count as newest and return it.
         cacheBlock.mLastTouchedCount = ++mRamCacheTouchCount;
         if( sbDebugPrintRamCache )
         {
            printf("HIT! GetRamCache[%d]: %s, %I64d (%I64d)\n", i, cacheBlock.mFilename, cacheBlock.mOffset, cacheBlock.mLastTouchedCount );
         }
         return &cacheBlock;
      }
   }

   //Not loaded.
   return NULL;
}

SArchiveRamCacheBlock * CArchiveReader::FindOldestFreeRamCacheBlock()
{
   int oldestCacheBlock = -1;
   unsigned __int64 oldestCacheBlockAge = 0xFFFFFFFFFFFFFFFF;
   for( int i=0; i < mRamCache.size(); ++i )
   {
      SArchiveRamCacheBlock & cacheBlock = mRamCache[i];

      if( cacheBlock.mLockCount == 0 && cacheBlock.mLastTouchedCount < oldestCacheBlockAge )
      {
         oldestCacheBlockAge = cacheBlock.mLastTouchedCount;
         oldestCacheBlock = i;
      }
   }

   //cache miss.  Return LRU so it can be replaced.
   if( oldestCacheBlock != -1 )
   {
      SArchiveRamCacheBlock & cacheBlock = mRamCache[oldestCacheBlock];
      return &cacheBlock;
   }

   //All cache blocks locked.
   return NULL;
}

void CArchiveReader::BeginLoadCacheBlock( SArchiveRamCacheBlock & cacheBlock, const char * const physicalFilename, const unsigned __int64 offset, const unsigned __int64 fileSize )
{
   BPE_ASSERT_NO_MSG( cacheBlock.mLockCount == 0 ); //not free!

   if( strcmp( cacheBlock.mFilename, physicalFilename ) != 0 )
   {
      //Different file.  Must (re)open this file.
      strcpy( cacheBlock.mFilename, physicalFilename );
      if( cacheBlock.mFileHandle != INVALID_HANDLE_VALUE )
      {
         CloseHandle(cacheBlock.mFileHandle);
      }
      cacheBlock.mFileHandle = CreateFile(physicalFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
      if( cacheBlock.mFileHandle == INVALID_HANDLE_VALUE )
      {
         printf("Error: Failed to open file! %s", physicalFilename );
         BPE_ASSERT_NO_MSG( false );
      }
   }

   //Round lower bound down to cache size alignment.
   cacheBlock.mOffset = offset & ( ~(unsigned __int64(skRamCacheBlockSize-1)) );
   cacheBlock.mSize = (unsigned int)bpe::min_val( (unsigned __int64)skRamCacheBlockSize, fileSize - cacheBlock.mOffset );
   cacheBlock.mFilledBytes = 0;

   memset( &cacheBlock.mOverlapped, 0, sizeof( cacheBlock.mOverlapped ) );

   LARGE_INTEGER largeFilePos = *(LARGE_INTEGER*)&cacheBlock.mOffset;
   cacheBlock.mOverlapped.Offset = largeFilePos.LowPart;
   cacheBlock.mOverlapped.OffsetHigh = largeFilePos.HighPart;
   unsigned int tryReadBytes = cacheBlock.mSize;
   //Round up to sector boundary for an async read.  didReadBytes will still be filled with the actual bytes read.
   unsigned int reqReadBytes = ( tryReadBytes + (skDvdSectorSize-1) ) & (~(skDvdSectorSize-1));
#ifndef GOLD_VERSION
   memset( cacheBlock.mData, 0xf3, skRamCacheBlockSize );
#endif
   int result = ::ReadFileEx(cacheBlock.mFileHandle, cacheBlock.mData, reqReadBytes, &cacheBlock.mOverlapped, NULL);
   int err = GetLastError();
   BPE_ASSERT( !( result == 0 && err != ERROR_IO_PENDING && err != ERROR_IO_INCOMPLETE ), "TODO: read error handling!" );

   cacheBlock.mLastTouchedCount = ++mRamCacheTouchCount;
   if( sbDebugPrintRamCache )
   {
      printf("MISS GetRamCache[%d]: %s, %I64d (%I64d)\n", &cacheBlock - &mRamCache[0], cacheBlock.mFilename, cacheBlock.mOffset, cacheBlock.mLastTouchedCount );
   }
}

void CArchiveReader::UpdateCacheBlock( SArchiveRamCacheBlock & cacheBlock )
{
   if( cacheBlock.mFilledBytes < cacheBlock.mSize )
   {
      //This cache block is currently loading.
      unsigned int didReadBytes = 0;
      BOOL result = GetOverlappedResult( cacheBlock.mFileHandle, &cacheBlock.mOverlapped, (LPDWORD)&didReadBytes, 0 );
      if( result )
      {
         //Completed successfully.
         unsigned int expectedReadBytes = ( cacheBlock.mSize + (skDvdSectorSize-1) ) & (~(skDvdSectorSize-1));
         BPE_ASSERT( didReadBytes <= expectedReadBytes, "read overrun" );
         if( ( didReadBytes == expectedReadBytes || didReadBytes == cacheBlock.mSize ) )
         {
            cacheBlock.mFilledBytes = cacheBlock.mSize;
         }
         else
         {
            //It's returned TRUE even though the overlapped operation is not complete.
            //Ignore until it reports the number of bytes we expect.
         }
      }
      else
      {
         int err = GetLastError();
         BPE_ASSERT( err == ERROR_IO_PENDING || err == ERROR_IO_INCOMPLETE, "TODO: read error handling!" ); //err will tell us what went wrong here.
      }
   }
}

//----------------------------------------------------------------------------

CArchiveFileHandle::CArchiveFileHandle()
{
   Reset();
}

void CArchiveFileHandle::Reset()
{
   mpArchive = NULL;
   mpTOCEntry = NULL;

   mVirtualCurrOffset = 0;
   mVirtualSize = 0;
   memset( mFilename, 0, sizeof( mFilename ) );

   mpOp = NULL;

   mbInUse = false;
}

unsigned __int64 CArchiveFileHandle::GetFileSize()
{
   return mVirtualSize;
}

int CArchiveFileHandle::Seek(const __int64 offset, const int mode)
{
   switch(mode)
   {
   case SEEK_SET:
      BPE_ASSERT_NO_MSG( offset >= 0 );
      mVirtualCurrOffset = offset;
      break;
   case SEEK_CUR:
      mVirtualCurrOffset += offset;
      break;
   case SEEK_END:
      mVirtualCurrOffset = GetFileSize() + offset;
      break;
   }
   mVirtualCurrOffset = bpe::min_val( bpe::max_val( (__int64)0, (__int64)mVirtualCurrOffset ), (__int64)mVirtualSize );
   return 0;
}

unsigned __int64 CArchiveFileHandle::Tell() const
{
   return mVirtualCurrOffset;
}

CArchiveOp::CArchiveOp( const unsigned int blockSize )
: mBlockBuffer( blockSize, 0 )
{
   Reset();
}

void CArchiveOp::Reset()
{
   mpFileHandle = NULL;
   mOffset = 0;
   mpBuf = NULL;
   mByteCount = 0;
   mbFileOwner = false;

   mpPendingCacheBlock = NULL;

   mCurrBlockIndex = 0;
   mCurrBlockOffset = 0;

   mWrittenBytes = 0;
   mVirtualReadBytes = 0;

   mBlockBufferFilledBytes = 0;

   mbInUse = false;
}

void CArchiveOp::DecompressionJobCompleteCallback( const SArchiveDecompressionJob * const pJob )
{
   //Update written bytes by the number of bytes that were copied out after decompression of this block.
   CArchiveOp * pOp = (CArchiveOp*)pJob->mUserParm;
   pOp->mWrittenBytes += pJob->mDecompressedSizeWithinBlock;
}
