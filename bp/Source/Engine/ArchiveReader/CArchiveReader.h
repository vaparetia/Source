//----------------------------------------------------------------------------
// CArchiveReader.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

#include <ctype.h>

#include "Engine/Mechanics/zlib/zlib.h"

#if BPE_TARGET == BPE_TARGET_X360

#include <xtl.h>

struct SArchiveHeader
{
   unsigned int   mMagicNumber;
   unsigned int   mVersionNumber;
   unsigned int   mFileCount;
   unsigned int   mBlockSize;
   unsigned int   mBlockCount;
};

struct SArchiveTOCEntry
{
   unsigned char mArchivePathHash[20];
   unsigned int mBlockListStartIndex;  //Index of first block in compressed block size list for this file
   unsigned int mOriginalSize;         //uncompressed size
   unsigned __int64 mStartOffset;      //Start offset of compressed blocks for this file in the archive

   bool operator < ( const unsigned char * rhs ) const
   {
      return memcmp( mArchivePathHash, rhs, sizeof(mArchivePathHash) ) < 0;
   }
};

struct SArchiveRamCacheBlock
{
   SArchiveRamCacheBlock();

   char              mFilename[FILENAME_MAX];
   unsigned __int64  mOffset;
   unsigned char *   mData;
   unsigned int      mSize;
   unsigned __int64  mLastTouchedCount;

   HANDLE            mFileHandle;
   OVERLAPPED        mOverlapped;
   unsigned int      mFilledBytes;
   int               mLockCount;
};

struct SArchiveData
{
   SArchiveData();
   ~SArchiveData();

   void Initialize( const char * const path );
   void StartCommonArchiveInitializeThread();
   void InitializeCommonArchive_Internal();

   typedef std::vector<SArchiveTOCEntry> TTOC;
   typedef std::vector<unsigned int> TBlockSizeVector;

   HANDLE                     mFileHandle;
   std::string                mFilePath;
   unsigned __int64           mArchiveSize;
   unsigned __int64           mArchiveFileModificationDateTime;
   SArchiveHeader             mHeader;
   TTOC                       mTOC;
   TBlockSizeVector           mCompressedBlockSizes;

   bool                       mIsCommonArchive;
   mutable bool               mIsInitCommonArchive; // if mIsCommonArchive then this gets set to true after init thread finishes
   std::vector<char>          mRamByteData; // This is an entire copy of the file, the data section is still compressed
   DWORD                      mArchiveInit_ThreadD;
   mutable HANDLE             mArchiveInit_ThreadHandle; // if mIsCommonArchive then this thread is created to initialize the archive (load data, copy into mRamByteData, etc...)
};

class CArchiveFileHandle;
class CArchiveOp;

class CArchiveReader
{
public:
   CArchiveReader();

   void Initialize();

   bool HasArc() const;
   void StartCommonArchiveInitializeThread();
   bool OpenArchive(const char * const path, bool isCommonArchive=false);
   int Shutdown();

   bool FileExists( const char * const path, const char * const commonPath ) const;
   long GetFileSizeAttr( const char * const path, const char * const commonPath ) const;

   CArchiveFileHandle * OpenFile(const char * const path, const char * const commonPath);
   void CloseFile(CArchiveFileHandle * pFileHandle);

   int ReadFile(CArchiveFileHandle * pFileHandle, void * pBuf, unsigned int byteCount);

   CArchiveOp * ReadFileAsync(CArchiveFileHandle * pFileHandle, void * pBuf, unsigned int byteCount);
   CArchiveOp * ReadFileAsync( const char * const path, void * pBuf, const unsigned int offset, unsigned int byteCount );

   bool IsOpComplete( CArchiveOp * pOp );
   void DeleteOp( CArchiveOp * pOp );

private:
   void UpdateOp( CArchiveOp * const pOp );

   void UpdateReadArchive( CArchiveOp * const pOp );
   void UpdateReadLoose( CArchiveOp * const pOp );

   int  TryReadRamCache( CArchiveOp * const pOp, const unsigned __int64 srcOffset, void * const pBuf, const unsigned int reqSize );

   SArchiveRamCacheBlock * FindRamCacheBlockThatContains( const char * const physicalFilename, const unsigned __int64 offset );
   SArchiveRamCacheBlock * FindOldestFreeRamCacheBlock();
   void BeginLoadCacheBlock( SArchiveRamCacheBlock & cacheBlock, const char * const physicalFilename, const unsigned __int64 offset, const unsigned __int64 fileSize );
   void UpdateCacheBlock( SArchiveRamCacheBlock & cacheBlock );

   const SArchiveTOCEntry * FindTOCEntry( const char * const path, const char * const commonPath, const SArchiveData ** const ppOutArchive ) const;
   const SArchiveTOCEntry * FindTOCEntry( const SArchiveData & archive, const char * const path ) const;

   const char *   GetPhysicalFileName( const CArchiveFileHandle * const pFileHandle ) const;

   CArchiveFileHandle * FindFreeFileHandle();
   CArchiveOp *         FindFreeFileOp();

private:
   typedef std::vector< SArchiveData >          TArchiveDataVector;
   typedef std::vector< SArchiveRamCacheBlock > TRamCache;

   typedef std::vector< CArchiveFileHandle * >  TFileHandleVector;
   typedef std::vector< CArchiveOp * >          TFileOpVector;

private:
   TArchiveDataVector         mArchives;
   TArchiveDataVector         mCommonArchives;
   std::string                mArchiveFolderPath;

   TFileHandleVector          mFileHandlePool;
   TFileOpVector              mFileOpPool;

   std::vector<unsigned char> mDecompressionBuffer;

   TRamCache                  mRamCache;
   unsigned __int64           mRamCacheTouchCount;

   XMEMDECOMPRESSION_CONTEXT  mDecompressionContext;
};

class CArchiveFileHandle
{
   friend class CArchiveOp;
   friend class CArchiveReader;
public:

   unsigned __int64 GetFileSize();
   int Seek(const __int64 offset, const int mode);
   unsigned __int64 Tell() const;

private:

   CArchiveFileHandle();
   void                       Reset();

private:
   const SArchiveData *       mpArchive;
   const SArchiveTOCEntry *   mpTOCEntry;

   unsigned __int64           mVirtualCurrOffset;
   unsigned __int64           mVirtualSize;
   char                       mFilename[FILENAME_MAX];

   CArchiveOp *               mpOp;

   bool                       mbInUse;
};

struct SArchiveDecompressionJob;

//only async-read ops supported for now!
class CArchiveOp
{
   friend class CArchiveReader;
public:

private:

   CArchiveOp( const unsigned int blockSize );
   void Reset();

   static void                DecompressionJobCompleteCallback( const SArchiveDecompressionJob * const pJob );

private:
   CArchiveFileHandle *       mpFileHandle;
   unsigned __int64           mOffset;          //start offset of read, corresponds to mVirtualCurrOffset of file handle when read starts.
   void *                     mpBuf;
   unsigned int               mByteCount;       //number of bytes to read
   bool                       mbFileOwner;      //if the op owns the file, close it automatically when deleted.

   SArchiveRamCacheBlock *    mpPendingCacheBlock; //cache block we're currently waiting on

   int                        mCurrBlockIndex;     //index of current compressed block being processed
   unsigned int               mCurrBlockOffset;    //accumulated offset in archive of current compressed block

   volatile unsigned int      mWrittenBytes;       //how many uncompressed bytes have been written to dest buffer
   unsigned int               mVirtualReadBytes;   //how many virtual bytes have been read from the archive

   std::vector<unsigned char> mBlockBuffer;              //buffer for a compressed block worth of data
   unsigned int               mBlockBufferFilledBytes;   //if a compressed block straddles a ram cache block, track size of the first read

   bool                       mbInUse;
};

#endif
