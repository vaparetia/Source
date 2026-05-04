//------------------------------------------------------------------------------------------
// CResourceFactoryLoadItem.h
// Bluepoint
// PS3 class for loading resources.
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Resource/IResourcePool.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/System/COsContext.h"
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
                              void ** ppResource,
                              CResourceFactory::SFactoryEntry const & factory,
                              IResourcePool * pResourcePool)
: CBaseResourceFactoryLoadItem(resource, ppResource, factory, pResourcePool)
, mFileDescriptor(-1)
, mAsyncIOHandle(-1)
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
   if (mFileDescriptor != -1)
   {
      CellFsErrno ret = 0;
      if (!mLoadCompleted)
      {
         if (mAsyncIOHandle != -1)
         {
            ret = cellFsAioCancel(mAsyncIOHandle);
         }
      }
      ret = cellFsClose(mFileDescriptor);
   }
}
      
//----------------------------------------------------------------------------
void CResourceFactoryLoadItem::AsyncLoadCallback(CellFsAio *aio, CellFsErrno err, int id, uint64_t size)
{
   //bpe_debugger_printf("callback is called id = %d\n", id);
   //bpe_debugger_printf("  err[%d] = 0x%x\n", id, err);
   //bpe_debugger_printf("  size[%d] = %llu\n", id, size);
   //bpe_debugger_printf("  buf[%d] = %s\n",id, (char *)aio->buf);
   //bpe_debugger_printf("  user_data[%d] = %llu\n",id, aio->user_data);
   CResourceFactoryLoadItem * pThis = reinterpret_cast<CResourceFactoryLoadItem*>(aio->user_data);
   pThis->mLoadCompleted = true;
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

   string const filePath = CResourceManager::GetLocalPath( mResourceId );
   CellFsErrno ret = cellFsOpen(filePath.c_str(), CELL_FS_O_RDONLY, &mFileDescriptor, NULL, 0);
   if (ret == CELL_FS_ENOENT)
   {
      // Try compressed version
      ret = cellFsOpen((filePath + ".gz").c_str(), CELL_FS_O_RDONLY, &mFileDescriptor, NULL, 0);
      mIsCompressed = true;
   }
   if (ret != CELL_FS_SUCCEEDED)
   {
      std::string const error(CStringExtras::Stringize("Asset: %s could not be loaded. Error: %x\n", filePath.c_str(), ret));
      bpe_debugger_printf("%s", error.c_str());
      BPE_VERIFYA( false, error.c_str() );
      *mpTargetResourcePtr = NULL;    
      return;
   }
   CellFsStat sb;
   ret = cellFsFstat(mFileDescriptor, &sb);
   mSize = (uint32) sb.st_size;
   mData.resize( mSize, 0 );

   // Setup async IO
   mAsyncIOInfo.buf = &mData[0];
   mAsyncIOInfo.fd = mFileDescriptor;
   mAsyncIOInfo.offset = 0;
   mAsyncIOInfo.size = (uint64_t) mSize;
   mAsyncIOInfo.user_data =  (uint64_t) this;

   ret = cellFsAioRead(&mAsyncIOInfo, &mAsyncIOHandle, CResourceFactoryLoadItem::AsyncLoadCallback);
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

   // Close ASAP to save on handles
   cellFsClose(mFileDescriptor);
   mFileDescriptor = -1;

   // Build
   {
      char *pData = &mData[0];
      uint32 size = mSize;

      // Used by decompress if compressed
      uint8 * pDestDataSysMemory = NULL;
      uint8 * pDestDataMalloc = NULL;

      if (mIsCompressed)
      {
         // Decompress
         // HACK! last 4 bytes of file are decompressed size with .gz
         // decompressed size in Intel format
         uint32 decompressedSize = *((uint32*) (pData + (size - 4)));                  
         Swap4Bytes_Always(&decompressedSize);  // Size stored in Intel endian

         uint8 * pDestData = NULL;
         // Try allocating decompression memory from memory container first to save on normal memory
         pDestDataSysMemory = OsContext()->AllocateTempMemory(decompressedSize);
         pDestData = pDestDataSysMemory;


         if (!pDestDataSysMemory)
         {
            pDestDataMalloc = (uint8*) malloc(decompressedSize);
            pDestData = pDestDataMalloc;
         }

         int const ret = uncompress_gzip(pDestData, &decompressedSize, (uint8*) pData, size);
         BPE_VERIFY(ret == 0, false, "zlib decompression error!");

         // Deallocate src data
         mData = std::vector<char>();
         pData = (char *) pDestData;
         size = decompressedSize;
      }

      SFactoryReturnResource returnResource;
      mFactory.BuildResource( mResourceId, pData, size, *mpResourcePool, returnResource );
      *mpTargetResourcePtr = returnResource.mpResource;

      // Deallocate uncompressed data
      if (pDestDataSysMemory)
      {
         // We used sys memory
         OsContext()->DeallocateTempMemory(pDestDataSysMemory);
      }
      else if (pDestDataMalloc)
      {
         // We used normal memory
         free(pDestDataMalloc);
      }
   }

   mIsDone = true;
}

//----------------------------------------------------------------------------

