//------------------------------------------------------------------------------------------
// CResourceFactoryLoadItem.h
// Bluepoint
// Win32 class for loading resources.
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Resource/X360CResourceFactoryLoadItem.h"
#include "Engine/Resource/CResourcePool.h"

//----------------------------------------------------------------------------

#include "Engine/System/CSystemVar.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/System/COsContext.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CResourceFactoryLoadItem::CResourceFactoryLoadItem(CResId const & resource,
                                                   void ** ppResource,
                                                   CResourceFactory::SFactoryEntry const & factory,
                                                   IResourcePool * pResourcePool)
                                                   :  CBaseResourceFactoryLoadItem(resource, ppResource, factory, pResourcePool)
                                                   ,  mSize( 0 )
{
   memset( &mOverlapped, 0, sizeof( OVERLAPPED ) );
   mOverlapped.hEvent = INVALID_HANDLE_VALUE;
   mFileHandle = INVALID_HANDLE_VALUE;
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

   if (HasAsyncLoadingStarted())
   {
      ::CloseHandle( mOverlapped.hEvent );
      ::CloseHandle( mFileHandle );
   }
}

//----------------------------------------------------------------------------

int32 CResourceFactoryLoadItem::GetAsyncLoadingFileSize() const
{
   if (mIsAsyncLoadingStarted)
   {
      return mSize;
   }

   // Don't know yet
   return 0;
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::StartAsyncLoad()
{
   mIsAsyncLoadingStarted = true;
   if (IsDone() || IsCancelled())
   {
      return;
   }

   // See if we've got a special case 'construct using path' resource.
   bool const bConstructFromMemory = (mFactory.mFlags & CResourceFactory::SFactoryEntry::kFF_ConstructUsingResourcePath) == 0;

   // f this factory constructs from memory, we need to load the data
   if ( bConstructFromMemory )
   {
      {
         string filePath = CResourceManager::GetLocalPath( mResourceId );

         mFileHandle = CreateFile( filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );

         if( mFileHandle == INVALID_HANDLE_VALUE )
         {
            // try compressed extension
            mFileHandle = CreateFile( (filePath + ".gz").c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );
            mIsCompressed = true;
         }

         if( mFileHandle != INVALID_HANDLE_VALUE )
         {
            mOverlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

            mSize = GetFileSize( mFileHandle, NULL );
            mData.resize( mSize, 0 );

            if( ReadFile( mFileHandle, &mData[0], mSize, NULL, &mOverlapped ) == TRUE )
            {
               // File IO finished, build it immediately
               // We can take this out later if we want to schedule cpu time for constructing objects
               Update();
            }
            else
            {
               BPE_VERIFY( GetLastError() == ERROR_IO_PENDING, false, "error other than pending io" );
            }
         }
         else
         {
            std::string const error("Couldn't find asset : " + filePath );
            if (!bpe_debugger_is_active())
            {
               // exit rather than crash!
               bpe_message_box( error.c_str(), "Missing asset!" );
               COsContext::TerminateProcess();
            }
            BPE_VERIFYA(false, error.c_str());
            *mpTargetResourcePtr = NULL;
            mIsDone = true;
         }
      }
   }
   else
   {
      // Special case 'resource path' case.
      {
         SFactoryReturnResource returnResource;
         mFactory.BuildResource( mResourceId, NULL, 0, *mpResourcePool, returnResource );
         *mpTargetResourcePtr = returnResource.mpResource;
         mIsDone = true;
      }
   }
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::Update()
{
   if (!HasAsyncLoadingStarted())
   {
      // Nothing to update
      return;
   }

   if( !mIsDone )
   {
      if( mFileHandle != INVALID_HANDLE_VALUE )
      {
         if( WaitForSingleObject( mOverlapped.hEvent, 0 ) == WAIT_OBJECT_0 )
         {
            if( !mCancelled )
            {
               char *pData = &mData[0];
               uint32 size = mSize;
               if (mIsCompressed)
               {
                  // Decompress
                  // HACK! last 4 bytes of file are decompressed size with .gz
                  // decompressed size in Intel format
                  uint32 decompressedSize = *((uint32*) (pData + (size - 4)));                  

                  uint8 * pDestData = (uint8*) malloc(decompressedSize);

                  int const ret = CResourceFactory::DecompressResource(pDestData, &decompressedSize, (uint8*) pData, size, mResourceId);
                  BPE_VERIFY(ret == 0, false, "zlib decompression error!");

                  // Deallocate src data
                  mData = std::vector<char>();
                  pData = (char *) pDestData;
                  size = decompressedSize;
               }

               SFactoryReturnResource returnResource;
               mFactory.BuildResource( mResourceId, pData, size, *mpResourcePool, returnResource );
               *mpTargetResourcePtr = returnResource.mpResource;

               if (mIsCompressed)
               {
                  // Deallocate uncompressed data
                  free(pData);
               }

               mIsDone = true;
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

