//------------------------------------------------------------------------------------------
// RVLCResourceFactoryLoadItem.h
// Bluepoint
// RVL class for loading resources.
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Resource/IResourcePool.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/RVL/RVLFileIO.h"

#include BPE_PLATFORM_SPECIFIC2(  Engine/Resource, CResourceFactoryLoadItem.h )

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

static void Swap4Bytes_Always( void * pData )
{
   char * pChar = (char *) pData;

   {
      char temp = pChar[0];
      pChar[0] = pChar[3];
      pChar[3] = temp;
   }

   {
      char temp = pChar[1];
      pChar[1] = pChar[2];
      pChar[2] = temp;
   }
}

//----------------------------------------------------------------------------
      
CResourceFactoryLoadItem::CResourceFactoryLoadItem( CResId const & resource,
                              void ** pObject,
                              CResourceFactory::SFactoryEntry const & factoryEntry,
                              IResourcePool * pResourcePool)
: CBaseResourceFactoryLoadItem(resource, pObject, factoryEntry, pResourcePool)
, mFileInfoValid( false )
, mSize(0)
, mLoadCompleted(false)
{
}
      
//----------------------------------------------------------------------------

CResourceFactoryLoadItem::~CResourceFactoryLoadItem()
{
   // Might have to watch out for threading issues later
   if (IsDone() && !IsCancelled())
   {
      // Notify resource pool of completion so it can trigger callbacks/caching
      ResourcePool()->NotifyResourceBuilt(GetResourceId());
   }

   if ( mFileInfoValid )
   {
      if ( !mLoadCompleted )
      {
         // Make sure the completed callback doesn't do anything
         RVLFileIO::Cancel( &mFileInfo );
      }
      RVLFileIO::Close( &mFileInfo );
   }
}
      
//----------------------------------------------------------------------------

int32 CResourceFactoryLoadItem::GetAsyncLoadingFileSize() const
{
   return mSize;
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::RVLFileIOCallback(s32 result, void *cookie, RVLFileIO::TFileInfo *fileInfo)
{
   CResourceFactoryLoadItem *pItem = reinterpret_cast< CResourceFactoryLoadItem *>( 
      cookie );

   if ( pItem )
   {
      pItem->mLoadCompleted = true;
   }
}

//----------------------------------------------------------------------------

bool CResourceFactoryLoadItem::OpenFile( char const *filePath, char const **ppErrorString )
{
   u32 errorCode = 0;
   bool openResult = RVLFileIO::OpenForRead( &mFileInfo, filePath, NULL );

   if ( !openResult )
   {
      char *gzFile = (char *) alloca( strlen( filePath ) + strlen( ".gz" ) + 1 );
      strcpy( gzFile, filePath );
      strcat( gzFile, ".gz" );
      openResult = RVLFileIO::OpenForRead( &mFileInfo, gzFile, NULL );
   }

   if ( !openResult )
   {
      *ppErrorString = "File not found";
      return false;
   }

   mSize = RVLFileIO::GetFileSize( &mFileInfo );
   mData.reset( reinterpret_cast<char *>( BPE_MALLOC_ALIGNED_MEM2( 32, OSRoundUp32B( mSize ) ) ) );
   mFileInfoValid = true;
   RVLFileIO::ReadAsyncCallback( &mFileInfo, mData.get(), mSize, &CResourceFactoryLoadItem::RVLFileIOCallback, this );

   return true;
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::StartAsyncLoad()
{
   mIsAsyncLoadingStarted = true;
   if (IsDone() || IsCancelled())
   {
      return;
   }

   string const filePath = CResourceManager::GetLocalPath( mResourceId );

   char const *errorCode = NULL;
   if ( !OpenFile( filePath.c_str(), &errorCode ) )
   {
      std::string const error(CStringExtras::Stringize("Asset: %s could not be loaded. Error: %s\n", filePath.c_str(), errorCode ));
      bpe_debugger_printf("%s", error.c_str());
      BPE_VERIFYA( false, error.c_str() );
      *mpTargetResourcePtr = NULL;    
      return;
   }
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::Update()
{
   if (IsDone() || IsCancelled())
   {
      return;
   }

   if (!HasAsyncLoadingStarted() || !mLoadCompleted)
   {
      // Waiting async start or for callback
      return;
   }

   // Close the handle if we're done with it
   if ( mFileInfoValid ) 
   {
      // Stop callbacks from doing anything
      RVLFileIO::Cancel( &mFileInfo );
      RVLFileIO::Close( &mFileInfo );

      mFileInfoValid = false;
   }

   // Build
   {
      char *pData = mData.get();
      uint32 size = mSize;

      // Used by decompress if compressed
      uint8 * pDestDataSysMemory = NULL;
      boost::scoped_ptr<uint8> pDestDataMalloc;

      if (mIsCompressed)
      {
         // Decompress
         // HACK! last 4 bytes of file are decompressed size with .gz
         // decompressed size in Intel format
         uint32 decompressedSize = *((uint32*) (pData + (size - 4)));                  
         Swap4Bytes_Always(&decompressedSize);  // Size stored in Intel endian

         uint8 * pDestData = NULL;
         pDestDataMalloc.reset( (uint8*) malloc(decompressedSize) );
         pDestData = pDestDataMalloc.get();

         int const ret = uncompress_gzip(pDestData, &decompressedSize, (uint8*) pData, size);
         BPE_VERIFY(ret == 0, false, "zlib decompression error!");

         // Deallocate src data
         mData.reset();
         pData = (char *) pDestData;
         size = decompressedSize;
      }

      SFactoryReturnResource ret;
      mFactory.BuildResource( mResourceId, pData, size, *mpResourcePool, ret );
      *mpTargetResourcePtr = ret.mpResource;
   }

   mIsDone = true;
}

//----------------------------------------------------------------------------

