//------------------------------------------------------------------------------------------
// CResourceFactory.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Mechanics/IObject.h"

#include "boost/shared_ptr.hpp"
#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"

class CInputStream;
class COutputStream;
class IResourceFactory;
class IResourcePool;
class IResourceCache;
class CBaseResourceFactoryLoadItem;
class CResId;
class CResourceReference;
class CResourceReference_Internal;
struct SFactoryReturnResource;

//------------------------------------------------------------------------------------------

class IResourceFactory
{
public:
   virtual ~IResourceFactory() {};

   virtual void      BuildSync(CResourceReference &resourceReference) = 0;
   virtual void      BuildAsync(CResourceReference *pResourceReference) = 0;           // Note that pResourceReference pointer must persist until load has finished.
   virtual void      CancelAsyncBuild(CResourceReference *pResourceReference) = 0;
};

//------------------------------------------------------------------------------------------
// Helper class to pass common const data to resource factory functions.
// Use MakeFactoryUserData template for construction.
// If you get a weird 
//    'use of undefined type 'boost::STATIC_ASSERTION_FAILURE<x>''
// compile error, it's because the template class  doesn't have a virtual destructor.

class CFactoryUserData
{
public:
   template <class T> static CFactoryUserData MakeFactoryUserData(T * pRetVal)
   {
      BOOST_STATIC_ASSERT(boost::is_polymorphic<T>::value);
      return CFactoryUserData(reinterpret_cast<IObject *>(pRetVal));
   }

   static CFactoryUserData MakeNull()
   {
      return CFactoryUserData(NULL);
   };

   template <class T> T *  get()        { return reinterpret_cast<T *>(get()); };
   IObject *               get()        { return mpData.get(); };

private:
   // Don't call this function! Use TFactoryUserData.
   CFactoryUserData(IObject * pUserData)
   : mpData(pUserData)
   {
   };

private:
   boost::shared_ptr<IObject>    mpData;
};

//------------------------------------------------------------------------------------------
// Helper struct for returning constructed assets.
// Resource system is responsible for memory allocation.
struct ENGINE_API SFactoryReturnResource
{
   SFactoryReturnResource();
   ~SFactoryReturnResource();

   void *   mpResource;
};


//------------------------------------------------------------------------------------------
// This helper struct contains required data for constructing resources.
struct SFactoryResourceBuildData
{
   SFactoryResourceBuildData(CResId const & resourceId, 
                             void * pMemory, 
                             int const size, 
                             IObject * const pFactoryUserData, 
                             IResourcePool &resourcePool)
      : mResourceId(resourceId)
      , mpMemory(pMemory)
      , mSize(size)
      , mpFactoryUserData(pFactoryUserData)
      , mResourcePool(resourcePool)
   {
   };

   CResId const &    mResourceId;
   void *            mpMemory;
   int const         mSize;
   IObject * const   mpFactoryUserData;   // Used for transferring per type factory data used in construction of resources (example would be an IGameObjectFactory)
   IResourcePool &   mResourcePool;
};

// Factory function typedef. Use this for factory functions.
typedef void (*TFactoryFn)(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

//------------------------------------------------------------------------------------------

class ENGINE_API CResourceFactory : public IResourceFactory
{
public:

   struct ENGINE_API SFactoryEntry
   {
      enum EFactoryFlags
      {
         kFF_ConstructUsingResourcePath      =  1 << 0,     // Set this flag when factory has to explicitly load asset using the path.
         kFF_NoVirtualDestructor             =  1 << 1,     // Set this flag when constructed asset is a binary blob allocated with malloc.

         kFF_None                            =  0,
         kFF_Invalid                         =  0xFFFFFFFF
      };

      explicit SFactoryEntry(uint32 const factoryFlags, TFactoryFn pFn);
      explicit SFactoryEntry(uint32 const factoryFlags, TFactoryFn pFn, CFactoryUserData &userData);

      void  BuildResource(CResId const & tag, 
                          void * pMemory, 
                          int const size, 
                          IResourcePool &resourcePool,
                          SFactoryReturnResource &returnResource);

      uint32                  mFlags;
      TFactoryFn              mpFactoryFn;
      CFactoryUserData        mpUserData; 
   };

   typedef std::map< uint32, SFactoryEntry > TFactoryMap;

public:
   CResourceFactory();
   virtual ~CResourceFactory();

   virtual void            BuildSync(CResourceReference &resourceReference);
   virtual void            BuildAsync(CResourceReference *pResourceReference);
   virtual void            CancelAsyncBuild(CResourceReference *pResourceReference);
   virtual bool            CanBuild(CResId const & resourceId);

   void                    AddFactory( uint32 const resIdExtension4CC, SFactoryEntry const &entry);
   void                    RemoveFactory( uint32 const resIdExtension4CC);

   void                    ProcessLoadingResources(real32 const maxCPUTime);

   // Run 'AssetTool /t' on every loaded asset.
   static void             SetAutoCookOnLoad(bool const bAutoCook);
   static bool             GetAutoCookOnLoad();

   // Print diagnostics about async loading
   enum EProfileLoading
   {
      kPL_None,
      kPL_Basic,
      kPL_Verbose,

      kPL_Count
   };
   static void             SetProfileLoading(EProfileLoading const profileType);
   static EProfileLoading  GetProfileLoading();

   void                    UpdateAsyncLoading();   // Monitors async loading count and starts new async loading
   static int              DecompressResource(uint8 * dest, uint32 *destLen, uint8 * const source, uint32 sourceLen, CResId const &resId);                  

private:
   TFactoryMap                      mFactories;
   std::list< boost::shared_ptr<CBaseResourceFactoryLoadItem> >  
                                    mLoadItems;

};

//------------------------------------------------------------------------------------------


