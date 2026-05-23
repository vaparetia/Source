//------------------------------------------------------------------------------------------
// CBaseGameManager.h
// Bluepoint
// CGameManager derives from this in gamecode.
// Allows CGameObjectComponent to pass a castable version game side to components.
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CRandom.h"
#include "Engine/Mechanics/IObject.h"
#include "Engine/GameObjectSystem/CGameObject.h"
#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "Engine/GameObjectSystem/CObjectList.h"
#include "boost/scoped_ptr.hpp"
#include "boost/weak_ptr.hpp"

//------------------------------------------------------------------------------------------

class IGameObjectFactory;
class CGameObjectMessageHandler;
class CGameObjectGlobalData;

//------------------------------------------------------------------------------------------
#if defined(__GNUC__) && !defined(_MSC_VER)
#include <ext/hash_map>
#define STD_HASH_MAP __gnu_cxx
#else
#include <hash_map>
#define STD_HASH_MAP stdext
#endif

// For use with STD_HASH_MAP::hash_map
class bgm_cguid_hash_compare
#if BPE_TARGET != BPE_TARGET_RVL && defined(_MSC_VER)
   : public STD_HASH_MAP::hash_compare<CGuid>
#endif
{
public:
   size_t operator() (CGuid const & guid) const
   {
      return guid.mUnionUint32_2;
   };

   bool operator() (CGuid const &lhs, CGuid const &rhs) const
   {
      return lhs < rhs;
   };   
};

//------------------------------------------------------------------------------------------

class CComponentEditorIdToPropertiesMapping
{
public:
   CComponentEditorIdToPropertiesMapping( TGameObjectUniqueId const &packageRootId,
                                          boost::shared_ptr<CGameObjectComponentProperties const> const &componentProperties,
                                          boost::shared_ptr<CGameObjectProperties const> const &objectProperties)
   :  mPackageRootId(packageRootId)
   ,  mComponentProperties(componentProperties)
   ,  mObjectProperties(objectProperties)
   {
   };

   struct SCompare
   {
      bool operator()(CComponentEditorIdToPropertiesMapping const &left, CComponentEditorIdToPropertiesMapping const &right);
   };

   TGameObjectUniqueId                                      mPackageRootId;
   boost::shared_ptr<CGameObjectComponentProperties const>  mComponentProperties;
   boost::shared_ptr<CGameObjectProperties const>           mObjectProperties;
};

//------------------------------------------------------------------------------------------
// Flags used to specify optional construction options
class ENGINE_API CGameObjectConstructionFlags
{
public:
   enum EFlags
   {
      kNone                                           = 0,
      kDontAddComponentEditorIdToPropertiesMapping    = 1 << 1,
      kAlwaysCreateOnLoad                             = 1 << 2,
      kDisableCreateOnLoad                            = 1 << 3,
      kDontConstructChildObjects                      = 1 << 4,
      kDontAddChildObjectsToNewObjectsList            = 1 << 5,
      kTerminator                                     = 0xFFFFFFFF
   };

   CGameObjectConstructionFlags(uint32 const flags = kNone)
      : mFlags(flags)
   {};
   ~CGameObjectConstructionFlags() {};

   uint32         mFlags;
};

//------------------------------------------------------------------------------------------
// This class is used to send params to component initialization (e.g. Send special parameters for transform on object generation)

class ENGINE_API CGameObjectConstructionParamEntry
{
public:
   CGameObjectConstructionParamEntry(uint32 const type, IObject const * pData, void const *pUserData)   // Takes ownership of pData, caller owns pUserData
      : mType(type)
      , mpData(pData)
      , mpUserData(pUserData)
   {
   };

   uint32                                 mType;
   boost::shared_ptr<IObject const>       mpData;
   void const *                           mpUserData;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectConstructionParams
{
public:
   typedef std::vector<CGameObjectConstructionParamEntry> TParams;

   CGameObjectConstructionParams();
   ~CGameObjectConstructionParams();

   boost::shared_ptr<IObject const>          FindDataByType(uint32 const type) const;
   void const *                              FindUserDataByType(uint32 const type) const;
   CGameObjectConstructionParamEntry const * FindParamEntryByType(uint32 const type) const;

   TParams              mParams;
};


//------------------------------------------------------------------------------------------

class ENGINE_API CBaseGameManager
{
public:
   // We should only have one properties entry per editor id/package root.
   typedef std::multimap<TComponentEditorId, CComponentEditorIdToPropertiesMapping>                                        TComponentEditorIdToPropertiesMap;     
   typedef std::pair<TComponentEditorIdToPropertiesMap::const_iterator, TComponentEditorIdToPropertiesMap::const_iterator> TComponentEditorIdToPropertiesMapConstIterPair;
   typedef std::pair<TComponentEditorIdToPropertiesMap::iterator, TComponentEditorIdToPropertiesMap::iterator>             TComponentEditorIdToPropertiesMapIterPair;

   typedef std::multimap<TComponentEditorId, CGameObjectComponent *>                                                       TComponentEditorIdToComponentMap;
   typedef std::pair<TComponentEditorIdToComponentMap::const_iterator, TComponentEditorIdToComponentMap::const_iterator>   TComponentEditorIdToComponentMapConstIterPair;
   typedef std::pair<TComponentEditorIdToComponentMap::iterator, TComponentEditorIdToComponentMap::iterator>               TComponentEditorIdToComponentMapIterPair;

   typedef STD_HASH_MAP::hash_map<TComponentUniqueId, CGameObjectComponent *, bgm_cguid_hash_compare>                      TComponentUniqueIdToComponentMap;

   typedef std::multimap<uint64, CGameObjectComponent *>                                                                         TComponentGroupAndTypeToComponentMap;
   typedef std::pair<TComponentGroupAndTypeToComponentMap::const_iterator, TComponentGroupAndTypeToComponentMap::const_iterator> TComponentGroupAndTypeToComponentMapConstIterPair;
   typedef std::pair<TComponentGroupAndTypeToComponentMap::iterator, TComponentGroupAndTypeToComponentMap::iterator>             TComponentGroupAndTypeToComponentMapIterPair;

   typedef std::map<uint64, boost::weak_ptr<CGameObjectGlobalData> >                                                       TGlobalDataMap;

   CBaseGameManager(IGameObjectFactory * const pGameObjectFactory, 
                    CGameObject * const pRootGameObject);   // Takes ownership
   virtual ~CBaseGameManager();

   // This must be called once per frame after everything else is done
   // One of the things done in here is flushing of pending object deletions
   // If this function is not called memory for objects will not be freed
   virtual void   UpdateGameManager();
   
   // This needs to be called before destruction to notify and delete game objects
   virtual void   ShutDown();

   CObjectList &           AllObjects()               { return mAllGameObjects; };
   CObjectList const &     GetAllObjects() const      { return mAllGameObjects; };

   CGameObject &           RootGameObject()           { return *mpRootGameObject; };
   CGameObject const &     GetRootGameObject() const  { return *mpRootGameObject; };

   CGameObject *           AddGameObject(CGameObject &parentObject,
                                         CGameObject &packageRootObject,
                                         boost::shared_ptr<CGameObjectProperties const> const & properties,
                                         IGameObjectFactory * const pFactory,
                                         CGameObjectConstructionFlags const &flags,
                                         CGameObjectConstructionParams const &params,
                                         std::vector<CGameObject *> * pNewGameObjects = NULL);

   void                    AddGameObjects(CGameObject &rootObject,
                                          CGameObject &packageRootObject,
                                          CGameObjectProperties::TChildGameObjectProperties const &propertiesArray,
                                          IGameObjectFactory * const pFactory,
                                          CGameObjectConstructionFlags const &flags,
                                          CGameObjectConstructionParams const &params,
                                          std::vector<CGameObject *> * pNewGameObjects = NULL);

   void                    DeleteObject(TGameObjectUniqueId const &id);

   CGameObject const *     GetGameObjectFromComponentUniqueId(TComponentUniqueId const &componentUniqueId) const;
   CGameObject *           GameObjectFromComponentUniqueId(TComponentUniqueId  const &componentUniqueId);
   CGameObjectComponent const *
                           GetComponentFromUniqueId(TComponentUniqueId  const &componentUniqueId) const;
   CGameObjectComponent *  ComponentFromUniqueId(TComponentUniqueId  const &componentUniqueId);

   CGameObjectComponent const *
                           GetFirstComponentFromUniqueOrEditorId(TComponentId const &componentId) const;
   CGameObjectComponent *  FirstComponentFromUniqueOrEditorId(TComponentId const &componentId);
   CGameObjectComponent const *
                           GetFirstComponentFromUniqueOrEditorId(TComponentId const &componentId, std::string const &query) const;
   CGameObjectComponent *  FirstComponentFromUniqueOrEditorId(TComponentId const &componentId, std::string const &query);

   // Returns the number of added components
   int                     GetComponentsFromUniqueOrEditorId(TComponentId const &componentId, TConstComponentListTransient &components) const;
   int                     ComponentsFromUniqueOrEditorId(TComponentId const &componentId, TComponentListTransient &components);

   bool                    ModifyComponentProperties( TComponentId const &componentEditorOrUniqueId,
                                                      CInputStream &propertiesStream,
                                                      bool const bCopyProperties);
   void                    ModifyComponentProperties( CGameObjectComponentProperties &componentProperties,
                                                      CInputStream &propertiesStream) const;
   CGameObjectMessageHandler & 
                           MessageHandler()  { return *mpMessageHandler; };

   CGameObjectMessageHandler const & 
                           GetMessageHandler()  { return *mpMessageHandler; };
   IGameObjectFactory *    GameObjectFactory()  { return mpGameObjectFactory; };

   CComponentEditorIdToPropertiesMapping const * const
                           GetComponentEditorIdToPropertiesMapping(TComponentEditorId const &editorId, TGameObjectUniqueId const &packageRootId) const;

   TComponentEditorIdToComponentMap const &
                           GetComponentEditorIdToComponentMap() const   { return mComponentEditorIdToComponentMap; };

   TComponentUniqueIdToComponentMap const &
                           GetComponentUniqueIdToComponentMap() const   { return mComponentUniqueIdToComponentMap; };

   TComponentUniqueIdToComponentMap &
                           ComponentUniqueIdToComponentMap()   { return mComponentUniqueIdToComponentMap; };

   TGlobalDataMap const &  GetGlobalDataMap() const   { return mGlobalDataMap; };
   TGlobalDataMap &        GlobalDataMap()            { return mGlobalDataMap; };

   void                    AddComponentUniqueIdToComponentMapping(TComponentUniqueId const &componentUniqueId, CGameObjectComponent * pComponent);
   void                    RemoveComponentUniqueIdToComponentMapping(TComponentUniqueId  const &componentUniqueId);
   // Helper functions to unload mapped properties.
   // These will unload any resources associated with editor ids if no more script objects exist.
   // Also, it will not be possible to generate these objects once unloaded.
   void                    RemoveComponentEditorToPropertiesMapping(CGameObjectProperties::TChildGameObjectProperties const &propertiesArray, TGameObjectUniqueId const &packageRootId);
   void                    RemoveComponentEditorToPropertiesMapping(boost::shared_ptr<CGameObjectProperties const> const & properties, TGameObjectUniqueId const &packageRootId);
   void                    RemoveComponentEditorToPropertiesMapping(TComponentEditorId const & editorId, TGameObjectUniqueId const &packageRootId);

   CRandom &   Random()                { return mRandom; };
   real64      GetUpdateTime() const   { return mUpdateTime; };
   uint32      GetUpdateCount() const  { return mUpdateCount; };
   uint32      GetRenderFrame() const  { return mRenderFrame; };
   uint32      GetInstanceCount() const{ return sInstanceCount; };

public:
   TComponentGroupAndTypeToComponentMap         mComponentGroupAndTypeToComponentMap;     // Used to help register global components - see GameObjectUtils

protected:
   // This function assume that the object to be added has already been parented off another game object.
   // Takes ownership of game object.
   void                    AddObject(CGameObject * const pObject);
   // This will free memory for any objects currently pending deletion
   void                    FlushPendingObjectDeletions();

protected:
   // Application is required to update these
   CRandom                       mRandom;
   real64                        mUpdateTime;
   uint32                        mUpdateCount;
   uint32                        mRenderFrame;
   uint32                        mNewObjectCount;  // Incremented each time an object is created.
   static uint32                 sInstanceCount;   // Incremented each time CBaseGameManager is created.

private:
   void                    AddComponentEditorIdToComponentMapping(CGameObject const * const pGO, 
                                                                  boost::shared_ptr<CGameObjectProperties const> const &goProperties,
                                                                  CGameObject const &packageRootObject);

   IGameObjectFactory *                         mpGameObjectFactory;
   CGameObject *                                mpRootGameObject;
   CObjectList                                  mAllGameObjects;
   TComponentEditorIdToPropertiesMap            mComponentEditorIdToPropertiesMap;        // Used to look up properties when generating objects
   TComponentEditorIdToComponentMap             mComponentEditorIdToComponentMap;         // Used to find components for messages and links
   TComponentUniqueIdToComponentMap             mComponentUniqueIdToComponentMap;         // Used to find components by unique id
   TGlobalDataMap                               mGlobalDataMap;
   boost::scoped_ptr<CGameObjectMessageHandler> mpMessageHandler;
   std::vector<CGameObject*>                    mObjectsToBeDeleted;
};

//------------------------------------------------------------------------------------------


