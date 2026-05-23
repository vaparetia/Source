//----------------------------------------------------------------------------
// CResourceManager.cpp
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CResourceManager.h"
#include "CResourceFactory.h"
#include "CResourcePool.h"
#include "CResourceCache.h"
#include "CBaseResourceFactoryLoadItem.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/TinyXml/tinyxml.h"
#include "Engine/Mechanics/TinyXml/xpath_processor.h"
#include "Engine/System/COsContext.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include <cell/cell_fs.h>
#endif

//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

namespace
{
   char skRepositoryPath[1024] = { -1, 0 };   // 0xFF indicates that repository hasn't been initialized.
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
class CResourceReference_Internal : public CResourceReference
{
public:
   enum EPersistentResource { kPersistentResource };
   CResourceReference_Internal(EPersistentResource)
      : CResourceReference(NULL)
   {
      // Add fake ref count so will never get deleted.
      mRefCnt++;
   }

   enum EInternalFlags
   {
      kIF_None                   = 0,
      kIF_Loading                = 1 << 0,
      kIF_NoVirtualDestructor    = 1 << 1,

      kIF_Terminator             = 0xFFFFFFFF
   };

   BPE_FORCEINLINE int   GetFlags(uint32 const getFlags) const    { return (mInternalFlags & getFlags); }  
   BPE_FORCEINLINE void  SetFlags(uint32 const setFlagsMask,
                                  uint32 const setFlagsState)
   { 
      mInternalFlags = (mInternalFlags & (~setFlagsMask)) | setFlagsState;
   }

};

//----------------------------------------------------------------------------

CResourceReference::CResourceReference(IObject * pResource,
                                       IResourcePool * pResourcePool,
                                       CResId const & resId )
:  mpResource( pResource )
,  mRefCnt( 0 )
,  mLockCnt( 0 )
,  mInternalFlags( 0 )
,  mpResourcePool( pResourcePool )
,  mResourceId( resId )
{
}

//----------------------------------------------------------------------------
// Resource with as blob of arbitrary data, allocated with 'malloc'
CResourceReference::CResourceReference(void * pResource,
                                       IResourcePool * pResourcePool,
                                       CResId const & resId )
:  mpResource( pResource )
,  mRefCnt( 0 )
,  mLockCnt( 0 )
,  mInternalFlags( 0 )
,  mpResourcePool( pResourcePool )
,  mResourceId( resId )
{
   RR_Internal().SetFlags(CResourceReference_Internal::kIF_NoVirtualDestructor, CResourceReference_Internal::kIF_NoVirtualDestructor);
}

//----------------------------------------------------------------------------
// Constructor for resource with virtual destructor, 'pResource' allocated with 'new' and 
// _not_managed by IResourcePool.

CResourceReference::CResourceReference(IObject * pResource)
:  mpResource( pResource )
,  mRefCnt( 0 )
,  mLockCnt( 0 )
,  mInternalFlags( 0 )
,  mpResourcePool( NULL )
,  mResourceId( CResId::Null() )
{
}

//----------------------------------------------------------------------------

CResourceReference::~CResourceReference()
{
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE CResourceReference_Internal & CResourceReference::RR_Internal()
{
   return static_cast<CResourceReference_Internal &>(*this);
}

//----------------------------------------------------------------------------

int CResourceReference::RemoveReference()
{
   --mRefCnt;

   if( !mRefCnt )
   {
      if( IsLoaded() )
      {
         Unload();
      }
      else if( RR_Internal().GetFlags(CResourceReference_Internal::kIF_Loading) )
      {
         mpResourcePool->ResourceFactory()->CancelAsyncBuild( this );
         RR_Internal().SetFlags(CResourceReference_Internal::kIF_Loading, 0);
      }
   }

   return mRefCnt;
}

//----------------------------------------------------------------------------

void CResourceReference::Lock()
{
   ++mLockCnt;

   if( !mpResource && !RR_Internal().GetFlags(CResourceReference_Internal::kIF_Loading) )
   {
      BPE_VERIFY(mpResourcePool != NULL, false, "Can't lock resource without IResourcePool.");
      mpResourcePool->ResourceFactory()->BuildAsync( this );
      // if BuildAsync already set mpObject to a valid pointer, it means it ended up being a sync build and we're done loading
      if( !mpResource )
      {
         RR_Internal().SetFlags(CResourceReference_Internal::kIF_Loading, CResourceReference_Internal::kIF_Loading);
      }
   }

}

//----------------------------------------------------------------------------

void CResourceReference::Unlock()
{
   --mLockCnt;

   if( !mLockCnt )
   {
      if( IsLoaded() )
      {
         Unload();
      }
      else if( RR_Internal().GetFlags(CResourceReference_Internal::kIF_Loading) )
      {
         mpResourcePool->ResourceFactory()->CancelAsyncBuild( this );
         RR_Internal().SetFlags(CResourceReference_Internal::kIF_Loading, 0);
      }
   }
}

//----------------------------------------------------------------------------

void * const CResourceReference::Resource()
{
   BPE_ASSERT( mRefCnt > 0, "must have reference" );
   BPE_ASSERT( mLockCnt > 0, "must be locked" );

   if( !mpResource )
   {
      mpResourcePool->ResourceFactory()->BuildSync( *this );
   }
   
   RR_Internal().SetFlags(CResourceReference_Internal::kIF_Loading, 0);

   return mpResource;
}

//----------------------------------------------------------------------------

void const * CResourceReference::GetResource() const
{
   BPE_ASSERT( mRefCnt > 0, "must have reference" );
   BPE_ASSERT( mLockCnt > 0, "must be locked" );

   // Call 'Object()' to make sure we build the object if not already built.
   return const_cast<CResourceReference*>(this)->Resource();
}

//----------------------------------------------------------------------------

void CResourceReference::Unload()
{
   BPE_ASSERT( IsLoaded(), "must be loaded" );
   BPE_ASSERT( mLockCnt == 0, "must not be locked" );

   if (RR_Internal().GetFlags(CResourceReference_Internal::kIF_NoVirtualDestructor))
   {
      // mpResource is just a lump of binary data, use 'free'
      free(mpResource);
   }
   else
   {
      // mpResource points to polymorphic class, use C++ delete.
      IObject *pResourceWithVirtualDestructor = reinterpret_cast<IObject *>(mpResource);
      delete pResourceWithVirtualDestructor;
   }
   mpResource = NULL;
   RR_Internal().SetFlags(CResourceReference_Internal::kIF_Loading, 0);
}

//----------------------------------------------------------------------------

void CResourceReference::Rebuild()
{
   if( IsLoaded() )
   {
      if (RR_Internal().GetFlags(CResourceReference_Internal::kIF_NoVirtualDestructor))
      {
         // mpResource is just a lump of binary data, use 'free'
         free(mpResource);
      }
      else
      {
         // mpResource points to polymorphic class, use C++ delete.
         IObject *pResourceWithVirtualDestructor = reinterpret_cast<IObject *>(mpResource);
         delete pResourceWithVirtualDestructor;
      }
      RR_Internal().SetFlags(CResourceReference_Internal::kIF_Loading, 0);
      mpResourcePool->ResourceFactory()->BuildSync( *this );
   }
}

//----------------------------------------------------------------------------

static CResourceReference_Internal sNullResource(CResourceReference_Internal::kPersistentResource);
CResourceReference * CResource::spNullReference = &sNullResource;

//----------------------------------------------------------------------------

CResourceWithDependencies::CResourceWithDependencies(CResource const &parentResource)
: mParentResource(parentResource)
, mLoaded(false)
, mLocked(false)
, mAddedDependencies(false)
{
   mDependencies.push_back(parentResource);
   // We'll 'AddDependencies' on Lock, otherwise we'll stall loading whilst checking dependencies.
}
   
//----------------------------------------------------------------------------

CResourceWithDependencies::~CResourceWithDependencies()
{
}

//----------------------------------------------------------------------------

void CResourceWithDependencies::AddDependencies(CResId const &resId) const
{
   if (mAddedDependencies)
   {
      // Don't need to recalculate the dependencies
      return;
   }

   mAddedDependencies = true;
   
   AddDependenciesRecursive(resId);
}

//----------------------------------------------------------------------------

void CResourceWithDependencies::AddDependenciesRecursive(CResId const &resId) const
{
   IResourcePool &pool = *mParentResource.ResourcePool();
   IResourcePool::TResourceChildDependencies const &deps = pool.GetChildDependencies(resId);

   for (IResourcePool::TResourceChildDependencies::const_iterator it = deps.begin(); it != deps.end(); ++it)
   {
      CResId const &depId = *it;
      bool bAlreadyAdded = false;
      
      // See if resource already exists in our dependency list (speed this up later)
      for (vector<CResource>::const_iterator findIt = mDependencies.begin(); findIt != mDependencies.end(); ++findIt)
      {
         CResId const &findResId = *findIt->GetResId();
         if (findResId == depId)
         {
            bAlreadyAdded = true;
            break;
         }
      }

      if (bAlreadyAdded) 
         continue;

      AddDependenciesRecursive(depId);

      // See if it exists NOW, since it might have been added while adding child dependencies recursivly.
      for (vector<CResource>::const_iterator findIt = mDependencies.begin(); findIt != mDependencies.end(); ++findIt)
      {
         CResId const &findResId = *findIt->GetResId();
         if (findResId == depId)
         {
            bAlreadyAdded = true;
            break;
         }
      }

      if(bAlreadyAdded)
         continue;

      mDependencies.push_back(pool.GetResource(depId));
   }
}

//----------------------------------------------------------------------------

void CResourceWithDependencies::Lock() const      
{
   if (!mLocked)
   {
      AddDependencies(*mParentResource.GetResId());

      for (vector<CResource>::const_iterator it = mDependencies.begin(); it != mDependencies.end(); ++it)
      {
         it->Lock();
      }
      mLocked = true;
   }
}

//----------------------------------------------------------------------------

void CResourceWithDependencies::Unlock()          
{
   if (mLocked)
   {
      for (vector<CResource>::iterator it = mDependencies.begin(); it != mDependencies.end(); ++it)
      {
         it->Unlock();
      }
      mLocked = false;
      mLoaded = false;
   }
}

//----------------------------------------------------------------------------

bool CResourceWithDependencies::IsLocked() const
{
   return mLocked;
}

//----------------------------------------------------------------------------

bool CResourceWithDependencies::IsLoading() const
{
   return mLocked && !IsLoaded();
}

//----------------------------------------------------------------------------

bool CResourceWithDependencies::IsLoaded() const
{
   if (!IsLocked()) return false;
   if (!mLoaded)
   {
      bool loaded = true;
      for (vector<CResource>::const_iterator it = mDependencies.begin(); it != mDependencies.end(); ++it)
      {
         if (!it->IsLoaded())
         {
            loaded = false;
            break;
         }
      }
      mLoaded = loaded;
   }
   return mLoaded;
}

//----------------------------------------------------------------------------
extern "C"
{
   char * gBPE_X360PlatformRoot = "D:\\";
   int gBPE_X360PlatformRootLength = 2;

   char const * BP_GetRootFilesystemPath();  // Used for PS3 root path mapping
}

static char * get_repository_path()
{
   if (skRepositoryPath[0] == -1)    // 0xFF indicates uninitialized path
   {
#if BPE_TARGET == BPE_TARGET_WIN32
      CSystemVar var( string("BPE_REPOSITORY") );
      if( var.IsValid() )
      {
         strcpy( skRepositoryPath, CFileUtils::UnifyPath(var.GetValue()).c_str() );
      }
#elif BPE_TARGET == BPE_TARGET_PS3
      // Path to files on HOSTFS
      // This can be overridden by CResourceManager::SetRepositoryPath
      strcpy(skRepositoryPath, BP_GetRootFilesystemPath() );
#elif BPE_TARGET == BPE_TARGET_X360
      memcpy(skRepositoryPath, gBPE_X360PlatformRoot, gBPE_X360PlatformRootLength);
      skRepositoryPath[gBPE_X360PlatformRootLength] = NULL;
#elif BPE_TARGET == BPE_TARGET_VITA
      strcpy( skRepositoryPath, COsContext::GetRootFolder() );
//      strcat( skRepositoryPath, "/" );
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
      strcpy(skRepositoryPath, "/cd");
#else
#  error Unknown platform!
      // Assume that we're at the repository root on other platforms
      strcpy(skRepositoryPath, "");
#endif
   }

   return skRepositoryPath;
}

//----------------------------------------------------------------------------

void CResourceManager::SetRepositoryPath(char const * const pNewRepositoryPath)
{
   // Mark it as initialized if there's actually a path, or uninitialized if not
   strcpy(skRepositoryPath, pNewRepositoryPath);
}

//----------------------------------------------------------------------------

char const * CResourceManager::GetRepositoryPath()
{
   return get_repository_path();
}

//----------------------------------------------------------------------------

std::string const CResourceManager::GetLocalPath( std::string const & assetPath )
{
   string result = CFileUtils::UnifyPath( assetPath );
   
   if( !result.empty() && result[0] == '$' )
   {
      result = string( get_repository_path() ) + BPE_PATH_SEPARATOR + result.substr( 2 );
   }
   else
   {
      // prepend base directory if necessary
      if( result.size() >= 2 )
      {
         if( result[1] != ':' )
         {
#if 0 //BPE_TARGET == BPE_TARGET_WIN32
            char buffer[1024];
            GetCurrentDirectory( 1024, buffer );
            result = string( buffer ) + result;
         }
         else
         {
#else
            BPE_ASSERT(false, "Invalid asset path");
#endif

         }
      }
   }

   return result;
}

//----------------------------------------------------------------------------

std::string const CResourceManager::GetAssetPath( std::string const & localPath )
{
   string result = CFileUtils::UnifyPath( localPath );
   
   if( !result.empty() && result[0] == '$' )
   {
      // Already an asset path, keep it
      result = localPath;
   }
   else
   {
      // Strip local repository path from it
      if( result.size() > strlen(get_repository_path()) )
      {
         if( result[1] == ':' )
         {  
            // Path from drive root
            result = result.substr(strlen(get_repository_path()));
            result = string("$") + result;
         }
         else
         {
            // Assume repository root relative
            result = string("$/") + result;
         }
      }
   }

   return result;
}

//----------------------------------------------------------------------------

#include "Engine/Mechanics/zlib/zlib.h"

//----------------------------------------------------------------------------

int uncompress_gzip_internal(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
   z_stream stream;
   int err;

   stream.next_in = (Bytef*)source;
   stream.avail_in = (uInt)sourceLen;
   /* Check for source > 64K on 16-bit machine: */
   if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

   stream.next_out = dest;
   stream.avail_out = (uInt)*destLen;
   if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

   stream.zalloc = (alloc_func)0;
   stream.zfree = (free_func)0;

   err = inflateInit2(&stream, 31); // Passing 31 makes it read the gzip header, not documented.
   if (err != Z_OK) return err;

   err = inflate(&stream, Z_FINISH);
   if (err != Z_STREAM_END) {
      inflateEnd(&stream);
      if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
         return Z_DATA_ERROR;
      return err;
   }
   *destLen = stream.total_out;

   err = inflateEnd(&stream);
   return err;
}

//----------------------------------------------------------------------------

int uncompress_gzip(uint8 * dest, uint32 *destLen, uint8 * const source, uint32 sourceLen)
{
   return uncompress_gzip_internal((Bytef*) dest, destLen, (Bytef*const) source, sourceLen);
}

//----------------------------------------------------------------------------

