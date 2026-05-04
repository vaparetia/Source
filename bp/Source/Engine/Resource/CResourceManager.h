//------------------------------------------------------------------------------------------
// CResourceManager.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/Resource/CResId.h"
#include "Engine/Mechanics/IObject.h"

class CInputStream;
class COutputStream;
class IResourceFactory;
class CResourceFactory;
class IResourcePool;
class IResourceCache;
class CBaseResourceFactoryLoadItem;
class CResourceFactoryLoadItem;
class CResourceReference_Internal;

//------------------------------------------------------------------------------------------

class ENGINE_API CResourceReference
{
   friend class CResourceFactory;
   friend class CResourceFactoryLoadItem;
public:
   // Constructor for resource with virtual destructor, managed by IResourcePool and 
   // 'pResource' allocated with 'new'.
   explicit CResourceReference( IObject * pResource,
                                IResourcePool *pResourcePool,
                                CResId const & resId );

   // Constructor for resource as blob of arbitrary data, 'pResource' allocated with 'malloc'
   explicit CResourceReference( void * pResource,
                                IResourcePool *pResourcePool,
                                CResId const & resId );

   // Constructor for resource with virtual destructor, 'pResource' allocated with 'new' and 
   // _not_ managed by IResourcePool.
   explicit CResourceReference(IObject * pResource);

   ~CResourceReference();

   void              AddReference()          { ++mRefCnt; };
   int               RemoveReference();
   void              Lock();
   void              Unlock();
   void * const      Resource();
   void const *      GetResource() const;
   bool              IsLoaded() const        { return mpResource != NULL; };

   void              Unload();
   CResId const &    GetResId() const        { return mResourceId; };
   IResourcePool *   ResourcePool() const    { return mpResourcePool; };
   int               GetLockCount() const    { return mLockCnt; };
   int               GetRefCount() const     { return mRefCnt; };

   // Advanced use only! Forces a rebuild on this object.
   void              Rebuild();

   // Internal helper function.
   CResourceReference_Internal & RR_Internal();

protected:
   // Use union for load item / resource as only one can be valid at a time.
   union
   {
      void *                           mpResource;    // NOTE: Can be 'void *' blob of arbitrary data or IObject, check mInternalFlags.
      CBaseResourceFactoryLoadItem *   mpLoadItem;
   };

   int               mRefCnt;
   int               mLockCnt;

   uint32            mInternalFlags;

   IResourcePool *   mpResourcePool;
   CResId            mResourceId;

private:
   BPE_DISABLE_OBJECT_ASSIGN( CResourceReference );
};

//------------------------------------------------------------------------------------------

class CResource
{
   // NULL static reference to simplify logic.
   ENGINE_API static CResourceReference * spNullReference;
   
public:
	enum EUninitializedResource { kUninitialized };

   CResource( CResource const & other )
   : mpResourceReference( other.mpResourceReference )
   , mLocked( false )
   {
      mpResourceReference->AddReference();
      if (other.IsLocked())
      {
         Lock();
      }
   }

   explicit CResource( CResourceReference * const pReference )
   : mpResourceReference( pReference )
   , mLocked( false )
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      mpResourceReference->AddReference();
   }

   // Use this to construct a uninitialized resource
   explicit CResource( EUninitializedResource )
   : mpResourceReference( spNullReference )
   , mLocked( false )
   {
      mpResourceReference->AddReference();
   }

   virtual ~CResource()
   {
      if( mLocked )
         Unlock();
      
		RemoveReference();
   }

   CResource & operator = ( CResource const & other )
   {
      if( mLocked )
         Unlock();
      
      RemoveReference();

      mpResourceReference = other.mpResourceReference;
      
      mpResourceReference->AddReference();
      
      if( other.IsLocked() )
         Lock();

      return *this;
   }

   bool operator ==( CResource const &other ) const
   {
      return mLocked == other.mLocked && mpResourceReference == other.mpResourceReference;
   }

   bool operator !=( CResource const &other ) const
   {
      return !( *this == other );
   }

   void Lock() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      if( !mLocked )
      {
         Lock_Internal();
      }
   }

   void Unlock()
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      
      if( mLocked )
      {
         mpResourceReference->Unlock();
         mLocked = false;
      }
   }

   bool IsLocked() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      return mLocked;
   }

   bool IsLoading() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      return mLocked && !IsLoaded();
   }

   bool IsLoaded() const
   { 
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      return mpResourceReference->IsLoaded();
   }

   bool IsNull() const { return mpResourceReference == NULL; }

   //---------------------
   // Advanced users only!

   // Use this to cast to known resource type.
   void const * const GetResource_Untyped() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      if (!mLocked)
      {
         Lock_Internal();
      }
      return mpResourceReference->GetResource();
   }
   
   // Use this to cast to known resource type.
   void * const Resource_Untyped()
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      if (!mLocked)
      {
         Lock_Internal();
      }
      
      return mpResourceReference->Resource();
   }

   // Use this with GetClassType to get the CResId for this token.
   CResourceReference const & GetResourceReference() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      return *mpResourceReference;
   }

   CResId const * GetResId() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      return &mpResourceReference->GetResId();
   }

   IResourcePool * ResourcePool() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      return mpResourceReference->ResourcePool();
   }

protected:
   // Used to get cached value in derived classes.
   void * const Resource_Internal() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      if (!mLocked)
      {
         Lock_Internal();
      }
      
      return mpResourceReference->Resource();
   }

private:

   void RemoveReference()
   {
      int const remainingReferences = mpResourceReference->RemoveReference();
      if( !remainingReferences )
      {
         delete mpResourceReference;
         mpResourceReference = NULL;
      }
   }

   void Lock_Internal() const
   {
      BPE_ASSERT( mpResourceReference != NULL, "Uninitialized CResource." );
      
      mpResourceReference->Lock();
      mLocked = true;
   }

private:
   CResourceReference*  mpResourceReference;
   mutable bool         mLocked;
};

//------------------------------------------------------------------------------------------

template <class T> class TResource : public CResource
{
public:
   TResource( CResource const & resource )
   : CResource( resource )
   {
   }

   // Explicit non-pool managed resource with virtual destructor.
   explicit TResource( T const * const pPtr )
   : CResource( new CResourceReference( (IObject*)pPtr ) )
   {
   }

   TResource( CResource::EUninitializedResource const &uninitialized)
   : CResource( uninitialized )
   {
   }

   BPE_FORCEINLINE bool operator ==( TResource const &other ) const
   {
      return static_cast< CResource const & >(*this) == static_cast<CResource const &>(other);
   }

   BPE_FORCEINLINE bool operator !=( TResource const &other ) const
   {
      return !( *this == other );
   }

   BPE_FORCEINLINE T const * const GetPtr() const
   { 
      T const * const pResource = reinterpret_cast<T * const>( Resource_Internal() );
      return pResource;
   }

   BPE_FORCEINLINE T * const Ptr()
   { 
      T * const pResource = reinterpret_cast<T * const>( Resource_Internal() );
      return pResource;
   }

   BPE_FORCEINLINE T * const operator -> ()
   {
      return Ptr();
   }

   BPE_FORCEINLINE T const * const operator -> () const
   {
      return GetPtr();
   }

   BPE_FORCEINLINE T & operator * ()
   {
      return *Ptr();
   }

   BPE_FORCEINLINE T const & operator * () const
   {
      return *GetPtr();
   }
};

//------------------------------------------------------------------------------------------

class CLockedResource : public CResource
{
public:
   CLockedResource( CResource const & resource )
   : CResource( resource )
   {
      Lock();
   }
};

//------------------------------------------------------------------------------------------

template <class T> class TLockedResource : public TResource<T>
{
public:
   explicit TLockedResource( TResource<T> const & resource )
   : TResource<T>( resource )
   {
      this->Lock();
   }

   explicit TLockedResource( CResource const & resource )
   : TResource<T>( resource )
   {
      this->TResource<T>::Lock();
   }
};

//------------------------------------------------------------------------------------------
// CResourceWithDependencies, used for loading assets needed by parent resource

class ENGINE_API CResourceWithDependencies
{
public:
   CResourceWithDependencies(CResource const &parentResource);
   virtual ~CResourceWithDependencies();
   
   void                    Lock() const;
   void                    Unlock();
   bool                    IsLocked() const;
   bool                    IsLoading() const;
   bool                    IsLoaded() const;

   // Use this to cast to known resource type.
   BPE_FORCEINLINE void * const           Resource_Untyped()               { return mParentResource.Resource_Untyped(); };
   BPE_FORCEINLINE void const * const     GetResource_Untyped() const      { return mParentResource.GetResource_Untyped(); };
   BPE_FORCEINLINE CResource const &      GetParentResource() const        { return mParentResource; };
   
protected:
   void                       AddDependencies(CResId const &resId) const;
   void                       AddDependenciesRecursive(CResId const &resId) const;

   CResource                        mParentResource;
   mutable std::vector<CResource>   mDependencies;       // Flat list of all child dependencies.
   mutable bool                     mLocked;             // Cached to speed up dependency checking.
   mutable bool                     mLoaded;             // Cached to speed up dependency checking.
   mutable bool                     mAddedDependencies;  // Cached to speed up dependency checking.
};

//------------------------------------------------------------------------------------------
template <class T> class TResourceWithDependencies : public CResourceWithDependencies
{
public:
   TResourceWithDependencies(CResourceWithDependencies const &resourceWithDeps)
   : CResourceWithDependencies(resourceWithDeps)
   {
   };
   
   TResourceWithDependencies(CResource const &parentResource)
   : CResourceWithDependencies(parentResource)
   {
   };
   
   BPE_FORCEINLINE T const * const GetPtr() const
   { 
      T const * const pResource = reinterpret_cast<T const * const>( GetResource_Untyped() );
      return pResource;
   }

   BPE_FORCEINLINE T * const Ptr()
   { 
      T * const pResource = reinterpret_cast<T * const>(const_cast<void * const>( GetResource_Untyped() ) );
      return pResource;
   }

   BPE_FORCEINLINE T * const operator -> ()
   {
      return Ptr();
   }

   BPE_FORCEINLINE T const * const operator -> () const
   {
      return GetPtr();
   }

   BPE_FORCEINLINE T & operator * ()
   {
      return *Ptr();
   }

   BPE_FORCEINLINE T const & operator * () const
   {
      return *GetPtr();
   }
};

//------------------------------------------------------------------------------------------

template <class T> class TLockedResourceWithDependencies : public TResourceWithDependencies<T>
{
public:
   TLockedResourceWithDependencies(CResourceWithDependencies const &resourceWithDeps)
   : TResourceWithDependencies<T>(resourceWithDeps)
   {
      this->Lock();
   };
   
   TLockedResourceWithDependencies(CResource const &parentResource)
   : TResourceWithDependencies<T>(parentResource)
   {
      this->Lock();
   };
   
   ~TLockedResourceWithDependencies()
   {
      this->Unlock();
   }
};

//------------------------------------------------------------------------------------------

class CResourceManager
{
public:
   ENGINE_API static std::string const GetLocalPath( std::string const & assetPath );     // Converts an asset path ('$/...') to local path ('k:/resources/...')
   ENGINE_API static std::string const GetAssetPath( std::string const & localPath );     // Converts an local patj ('k:/resources/..') to an asset path ('$/...')

   // PS3 only, used to override default '/app_home' directory to specified path.
   ENGINE_API static void           SetRepositoryPath(char const * const pNewRepositoryPath);
   ENGINE_API static char const *   GetRepositoryPath();
};

//------------------------------------------------------------------------------------------

ENGINE_API int uncompress_gzip(uint8 * dest, uint32 *destLen, uint8 * const source, uint32 sourceLen);

//------------------------------------------------------------------------------------------

