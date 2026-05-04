//----------------------------------------------------------------------------
// CBaseGameManager.cpp
// Bluepoint
// CGameManager derives from this in gamecode.
// Allows CGameObjectComponent to pass a castable version game side to components.
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"
#include "Engine/GameObjectSystem/CGameObjectPropertiesPackage.h"
#include "Engine/GameObjectSystem/CGameObjectPropertiesPackageEntry.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"
#include "Engine/GameObjectSystem/CGameObjectMessageHandler.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"
#include "Engine/GameObjectSystem/CGameObjectGlobalData.h"

//----------------------------------------------------------------------------

CGameObjectConstructionParams::CGameObjectConstructionParams()
{
}

//----------------------------------------------------------------------------

CGameObjectConstructionParams::~CGameObjectConstructionParams()
{
}

//----------------------------------------------------------------------------

boost::shared_ptr<IObject const> CGameObjectConstructionParams::FindDataByType(uint32 const type) const
{
   CGameObjectConstructionParamEntry const * pParam = FindParamEntryByType(type);
   if (pParam)
   {
      return pParam->mpData;
   }
   // Not found
   return boost::shared_ptr<IObject const>();
};

//----------------------------------------------------------------------------

void const * CGameObjectConstructionParams::FindUserDataByType(uint32 const type) const
{
   CGameObjectConstructionParamEntry const * pParam = FindParamEntryByType(type);
   if (pParam)
   {
      return pParam->mpUserData;
   }
   // Not found
   return NULL;
}

//----------------------------------------------------------------------------

CGameObjectConstructionParamEntry const * CGameObjectConstructionParams::FindParamEntryByType(uint32 const type) const
{
   for (TParams::const_iterator it = mParams.begin(); it != mParams.end(); ++it)
   {
      CGameObjectConstructionParamEntry const &entry =  *it;
      if (entry.mType == type)
      {
         return &entry;
      }
   }
   // Not found
   return NULL;
}

//----------------------------------------------------------------------------

uint32 CBaseGameManager::sInstanceCount = 0;

//----------------------------------------------------------------------------

CBaseGameManager::CBaseGameManager(IGameObjectFactory * const pGameObjectFactory, CGameObject * const pRootGameObject)
: mpGameObjectFactory(pGameObjectFactory)
, mpRootGameObject(pRootGameObject)
, mpMessageHandler(new CGameObjectMessageHandler(*this))
, mRandom(99)
, mUpdateTime(0.0)
, mUpdateCount(0)
, mRenderFrame(0)
, mNewObjectCount(0)
{
   sInstanceCount++;
   // Calling initialize will give the root object a unique id
   mpRootGameObject->Initialize(*this, kInvalidGameObjectUniqueId, CGameObjectConstructionParams());
   AddObject(mpRootGameObject);
}

//----------------------------------------------------------------------------

CBaseGameManager::~CBaseGameManager()
{
   FlushPendingObjectDeletions();
}

//----------------------------------------------------------------------------

void CBaseGameManager::UpdateGameManager()
{
   FlushPendingObjectDeletions();
}

//----------------------------------------------------------------------------

void CBaseGameManager::ShutDown()
{
   // Notify all objects of deletion
   // Deleting the root object will do this as all objects are parented off the root
   DeleteObject(mpRootGameObject->GetId());
   BPE_ASSERT(mAllGameObjects.GetObjectCount() == 0, "Stray game objects detected.");
}

//----------------------------------------------------------------------------

void CBaseGameManager::AddObject(CGameObject * const pObject)
{
   mAllGameObjects.AddObject(pObject);
}

//----------------------------------------------------------------------------

void CBaseGameManager::DeleteObject(TGameObjectUniqueId const &id)
{
   CGameObject * const pObject = mAllGameObjects.ObjectById(id);
   if (!pObject) return;

   pObject->Uninitialize(*this);
   mAllGameObjects.RemoveObject(id);

   mObjectsToBeDeleted.push_back(pObject);
}

//----------------------------------------------------------------------------

void CBaseGameManager::FlushPendingObjectDeletions()
{
   foreach(CGameObject *pGO, mObjectsToBeDeleted)
   {
      delete pGO;
   }
   
   bpe::clear_keep_capacity(mObjectsToBeDeleted);
}

//----------------------------------------------------------------------------

void CBaseGameManager::AddGameObjects( CGameObject &rootObject,
                                       CGameObject &packageRootObject,
                                       CGameObjectProperties::TChildGameObjectProperties const &propertiesArray,
                                       IGameObjectFactory * const pFactory,
                                       CGameObjectConstructionFlags const &flags,
                                       CGameObjectConstructionParams const &params,
                                       std::vector<CGameObject *> * pNewGameObjects)
{
   for (CGameObjectProperties::TChildGameObjectProperties::const_iterator it = propertiesArray.begin(); it != propertiesArray.end(); ++it)
   {
      boost::shared_ptr<CGameObjectProperties const>  goProperties = *it;
      CGameObject *pNewGameObject = AddGameObject(rootObject, packageRootObject, goProperties, pFactory, flags, params, pNewGameObjects);
   }
}

//----------------------------------------------------------------------------

CGameObject * CBaseGameManager::AddGameObject(CGameObject &parentObject,
                                              CGameObject &packageRootObject,
                                              boost::shared_ptr<CGameObjectProperties const> const & properties,
                                              IGameObjectFactory * const pFactory,
                                              CGameObjectConstructionFlags const &flags,
                                              CGameObjectConstructionParams const &params,
                                              std::vector<CGameObject *> * pNewGameObjects)
{
   // Create on load is normally set to false for objects to be generated
   bool bCreateOnLoad = (properties->mFlags & CGameObjectProperties::kLF_DontCreateOnLoad) == 0;
   if ((flags.mFlags & CGameObjectConstructionFlags::kAlwaysCreateOnLoad) != 0) bCreateOnLoad = true;
   if ((flags.mFlags & CGameObjectConstructionFlags::kDisableCreateOnLoad) != 0) bCreateOnLoad = false;

   CGameObject * pGO = NULL;
   if (bCreateOnLoad)
   {
      pGO = pFactory->BuildGameObject(properties);
      pGO->Initialize(*this, packageRootObject.GetId(), params);
      AddObject(pGO);

      parentObject.AddChild(pGO);
   }

   if ((flags.mFlags & CGameObjectConstructionFlags::kDontAddComponentEditorIdToPropertiesMapping) == 0)
   {
      // Add component id mapping even if object wasn't created
      AddComponentEditorIdToComponentMapping(pGO, properties, packageRootObject);
   }

   if ((flags.mFlags & CGameObjectConstructionFlags::kDontConstructChildObjects) == 0)
   {
      CGameObjectConstructionFlags newFlags(flags);
      // Disable child game object construction if parent wasn't created
      if (!bCreateOnLoad) newFlags.mFlags |= CGameObjectConstructionFlags::kDisableCreateOnLoad;

      // Remove kAlwaysCreate on load, otherwise children of generated objects will always be created, even if they are flagged not to.
      newFlags.mFlags &= ~CGameObjectConstructionFlags::kAlwaysCreateOnLoad;

      // Even if we're not going to create these game objects, we still need to add the editor id to component mapping
      std::vector<CGameObject *> * pNewGameObjectsLocal = (flags.mFlags & CGameObjectConstructionFlags::kDontAddChildObjectsToNewObjectsList) ? NULL : pNewGameObjects;
      AddGameObjects(*pGO, packageRootObject, properties->mChildGameObjectProperties, pFactory, newFlags, params, pNewGameObjectsLocal);
   }
   
   // Keep track of the objects added
   if (pNewGameObjects && pGO) pNewGameObjects->push_back(pGO);
   if (pGO) mNewObjectCount++;

   return pGO;
}

//----------------------------------------------------------------------------

CGameObject const * CBaseGameManager::GetGameObjectFromComponentUniqueId(TComponentUniqueId const &componentUniqueId) const
{
   TComponentUniqueIdToComponentMap::const_iterator it = mComponentUniqueIdToComponentMap.find(componentUniqueId);
   if (it != mComponentUniqueIdToComponentMap.end())
   {
      CGameObjectComponent const * pComponent = (*it).second;  // Result should never be null
      return pComponent->GetGameObject();
   }
   return NULL;
}

//----------------------------------------------------------------------------
CGameObject * CBaseGameManager::GameObjectFromComponentUniqueId(TComponentUniqueId const &componentUniqueId)
{
   TComponentUniqueIdToComponentMap::const_iterator it = mComponentUniqueIdToComponentMap.find(componentUniqueId);
   if (it != mComponentUniqueIdToComponentMap.end())
   {
      CGameObjectComponent * pComponent = (*it).second;  // Result should never be null
      return pComponent->GameObject();
   }
   return NULL;
}

//----------------------------------------------------------------------------
CGameObjectComponent const * CBaseGameManager::GetComponentFromUniqueId(TComponentUniqueId const &componentUniqueId) const
{
   TComponentUniqueIdToComponentMap::const_iterator it = mComponentUniqueIdToComponentMap.find(componentUniqueId);
   if (it != mComponentUniqueIdToComponentMap.end())
   {
      CGameObjectComponent const * pComponent = (*it).second;  // Result should never be null
      return pComponent;
   }
   return NULL;
}
//----------------------------------------------------------------------------

CGameObjectComponent * CBaseGameManager::ComponentFromUniqueId(TComponentUniqueId const &componentUniqueId)
{
   TComponentUniqueIdToComponentMap::const_iterator it = mComponentUniqueIdToComponentMap.find(componentUniqueId);
   if (it != mComponentUniqueIdToComponentMap.end())
   {
      CGameObjectComponent * pComponent = (*it).second;  // Result should never be null
      return pComponent;
   }
   return NULL;
}

//----------------------------------------------------------------------------

CGameObjectComponent const * CBaseGameManager::GetFirstComponentFromUniqueOrEditorId(TComponentId const &componentId) const
{
   return const_cast<CBaseGameManager *>(this)->FirstComponentFromUniqueOrEditorId(componentId);
}

//----------------------------------------------------------------------------

CGameObjectComponent * CBaseGameManager::FirstComponentFromUniqueOrEditorId(TComponentId const &componentId)
{
   if (componentId.IsUniqueId())
   {
      CGameObjectComponent * pComponent = ComponentFromUniqueId(componentId);
      return pComponent;
   }
   // Must be an editor id
   CGameObjectComponent * pComponent = NULL;
   CBaseGameManager::TComponentEditorIdToComponentMap const &map = GetComponentEditorIdToComponentMap();
   CBaseGameManager::TComponentEditorIdToComponentMapConstIterPair componentsIt = map.equal_range(componentId);
   for (CBaseGameManager::TComponentEditorIdToComponentMap::const_iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
   {
      pComponent = (*iter).second;
      break;
   }

   return pComponent;
}

//----------------------------------------------------------------------------

CGameObjectComponent const * CBaseGameManager::GetFirstComponentFromUniqueOrEditorId(TComponentId const &componentId, std::string const &query) const
{
   return const_cast<CBaseGameManager *>(this)->FirstComponentFromUniqueOrEditorId(componentId, query);
}

//----------------------------------------------------------------------------

CGameObjectComponent * CBaseGameManager::FirstComponentFromUniqueOrEditorId(TComponentId const &componentId, std::string const &query)
{
   if (componentId.IsUniqueId())
   {
      CGameObjectComponent * pComponent = ComponentFromUniqueId(componentId);
      if (pComponent && (query.length() != 0) && !pComponent->RunQuery(query))
      {
         pComponent = NULL;
      }

      return pComponent;
   }

   // Must be an editor id
   CGameObjectComponent * pComponent = NULL;
   CBaseGameManager::TComponentEditorIdToComponentMap const &map = GetComponentEditorIdToComponentMap();
   CBaseGameManager::TComponentEditorIdToComponentMapConstIterPair componentsIt = map.equal_range(componentId);
   for (CBaseGameManager::TComponentEditorIdToComponentMap::const_iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
   {
      pComponent = (*iter).second;
      if (query.size() == 0) break;
      if (pComponent->RunQuery(query)) break;
      pComponent = NULL;
   }
   
   return pComponent;
}

//----------------------------------------------------------------------------

int CBaseGameManager::GetComponentsFromUniqueOrEditorId(TComponentId const &componentId, TConstComponentListTransient &components) const
{
   int count = 0;
   if (componentId.IsUniqueId())
   {
      CGameObjectComponent const * pComponent = GetComponentFromUniqueId(componentId);
      if (pComponent)
      {
         components.reserve(1);
         components.push_back(pComponent);
         count++;
      }
   }
   else
   {
      // Must be an editor id
      CGameObjectComponent const * pComponent = NULL;
      CBaseGameManager::TComponentEditorIdToComponentMap const &map = GetComponentEditorIdToComponentMap();
      CBaseGameManager::TComponentEditorIdToComponentMapConstIterPair componentsIt = map.equal_range(componentId);
      for (CBaseGameManager::TComponentEditorIdToComponentMap::const_iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
      {
         pComponent = (*iter).second;
         components.push_back(pComponent);
         count++;
      }
   }
   
   return count;
}

//----------------------------------------------------------------------------

int CBaseGameManager::ComponentsFromUniqueOrEditorId(TComponentId const &componentId, TComponentListTransient &components)
{
   int count = 0;
   if (componentId.IsUniqueId())
   {
      CGameObjectComponent * pComponent = ComponentFromUniqueId(componentId);
      if (pComponent)
      {
         components.reserve(1);
         components.push_back(pComponent);
         count++;
      }
   }
   else
   {
      // Must be an editor id
      CGameObjectComponent * pComponent = NULL;
      CBaseGameManager::TComponentEditorIdToComponentMap const &map = GetComponentEditorIdToComponentMap();
      CBaseGameManager::TComponentEditorIdToComponentMapConstIterPair componentsIt = map.equal_range(componentId);
      for (CBaseGameManager::TComponentEditorIdToComponentMap::const_iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
      {
         pComponent = (*iter).second;
         components.push_back(pComponent);
         count++;
      }
   }

   return count;
}

//----------------------------------------------------------------------------

void CBaseGameManager::AddComponentUniqueIdToComponentMapping(TComponentUniqueId const &componentUniqueId, CGameObjectComponent * pComponent)
{
   mComponentUniqueIdToComponentMap.insert(TComponentUniqueIdToComponentMap::value_type(componentUniqueId, pComponent));
   TComponentEditorId const &editorId = pComponent->GetEditorId();
   // Check for NULL editor id as some code generated objects won't have an editor id (DebugCamera, root object);
   if (editorId.valid())
   {
      mComponentEditorIdToComponentMap.insert(TComponentEditorIdToComponentMap::value_type(pComponent->GetEditorId(), pComponent));
   }
}

//----------------------------------------------------------------------------

void CBaseGameManager::RemoveComponentUniqueIdToComponentMapping(TComponentUniqueId const &componentUniqueId)
{
   TComponentUniqueIdToComponentMap::const_iterator iter = mComponentUniqueIdToComponentMap.find(componentUniqueId);
   BPE_ASSERT(iter != mComponentUniqueIdToComponentMap.end(), "Can't removed already deleted component");
	CGameObjectComponent * pComponent = (*iter).second;
	BPE_ASSERT(pComponent != NULL, "Can't removed already deleted component");
	
   // Check for NULL editor id as some code generated objects won't have an editor id (DebugCamera, root object);
   TComponentEditorId const &editorId = pComponent->GetEditorId();
   if (editorId.valid())
   {
      TComponentEditorIdToComponentMapIterPair componentsIt = mComponentEditorIdToComponentMap.equal_range(editorId);
      for (TComponentEditorIdToComponentMap::iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
      {
         CGameObjectComponent * pComponent = (*iter).second;
         if (pComponent->GetId() == componentUniqueId)
         {
            // Only one component per unique id
            mComponentEditorIdToComponentMap.erase(iter);
            break;
         }
      }
   }
   mComponentUniqueIdToComponentMap.erase(componentUniqueId);
}


//----------------------------------------------------------------------------

void CBaseGameManager::RemoveComponentEditorToPropertiesMapping(CGameObjectProperties::TChildGameObjectProperties const &propertiesArray, TGameObjectUniqueId const &packageRootId)
{
   for (CGameObjectProperties::TChildGameObjectProperties::const_iterator it = propertiesArray.begin(); it != propertiesArray.end(); ++it)
   {
      boost::shared_ptr<CGameObjectProperties const>  goProperties = *it;
      RemoveComponentEditorToPropertiesMapping(goProperties, packageRootId);
   }
}

//----------------------------------------------------------------------------

void CBaseGameManager::RemoveComponentEditorToPropertiesMapping(boost::shared_ptr<CGameObjectProperties const> const & properties, TGameObjectUniqueId const &packageRootId)
{

   CGameObjectProperties::TComponentProperties const &componentPropertiesArray = properties->mComponentProperties;
   for (CGameObjectProperties::TComponentProperties::const_iterator iter = componentPropertiesArray.begin(); iter != componentPropertiesArray.end(); ++iter)
   {
      boost::shared_ptr<CGameObjectComponentProperties const> pComponentProperties = *iter;
      RemoveComponentEditorToPropertiesMapping(pComponentProperties->mEditorId, packageRootId);
   }
   RemoveComponentEditorToPropertiesMapping(properties->mChildGameObjectProperties, packageRootId);
}

//----------------------------------------------------------------------------

void CBaseGameManager::RemoveComponentEditorToPropertiesMapping(TComponentEditorId const & editorId, TGameObjectUniqueId const &packageRootId)
{
   // Find editor id and package root match so we can remove mapping
   TComponentEditorIdToPropertiesMapIterPair componentsIt = mComponentEditorIdToPropertiesMap.equal_range(editorId);
   for (TComponentEditorIdToPropertiesMap::iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
   {
      CComponentEditorIdToPropertiesMapping const & mapping = (*iter).second;
      if (mapping.mPackageRootId == packageRootId)
      {
         // That's the one, remove it
         // It's holding onto a smart ptr to properties, 
         // if nothing else is using it, properties (and associated resources) will be deleted.
         mComponentEditorIdToPropertiesMap.erase(iter);  
         break;
      }
   }
}

//----------------------------------------------------------------------------

void CBaseGameManager::AddComponentEditorIdToComponentMapping( CGameObject const * const pGO,
                                                               boost::shared_ptr<CGameObjectProperties const> const &goProperties, 
                                                               CGameObject const &packageRootObject)
{
   CGameObjectProperties::TComponentProperties const &componentProperties = goProperties->mComponentProperties;
   for (CGameObjectProperties::TComponentProperties::const_iterator iter = componentProperties.begin();
      iter != componentProperties.end();
      ++iter)
   {
      boost::shared_ptr<CGameObjectComponentProperties const> const &componentProperties = *iter;

      TComponentEditorId const &editorId = componentProperties->mEditorId;
      // Check for NULL editor id as some code generated objects won't have an editor id (DebugCamera, root object);
      if (editorId.valid())
      {
         mComponentEditorIdToPropertiesMap.insert(TComponentEditorIdToPropertiesMap::value_type(editorId, 
          CComponentEditorIdToPropertiesMapping(packageRootObject.GetId(), componentProperties, goProperties)));
      }
   }
}
//----------------------------------------------------------------------------
// Need to split this function into 2 types,
// one that takes a stream and only modifies unduplicated properties
// and another that takes a vector and can modify properties multiple times.
bool CBaseGameManager::ModifyComponentProperties(TComponentId const &componentEditorOrUniqueId, 
                                                 CInputStream &propertiesStream,
                                                 bool const bCopyProperties)
{
   if (mComponentEditorIdToPropertiesMap.find(componentEditorOrUniqueId) != mComponentEditorIdToPropertiesMap.end())
   {
      // It's an editor id
      if (!bCopyProperties)
      {
         // Update properties
         CComponentEditorIdToPropertiesMapping const * const pMapping = GetComponentEditorIdToPropertiesMapping(componentEditorOrUniqueId, kInvalidGameObjectUniqueId);
         if (pMapping)
         {
            // const cast! (special case, we're modifing const resource data shared between components)
            CGameObjectComponentProperties &componentProperties = *const_cast<CGameObjectComponentProperties*>(pMapping->mComponentProperties.get());
            ModifyComponentProperties(componentProperties, propertiesStream);
         }
      }
      
      // Inform components of change
      TComponentEditorIdToComponentMapConstIterPair componentsIter = mComponentEditorIdToComponentMap.equal_range(componentEditorOrUniqueId);
      for (TComponentEditorIdToComponentMap::const_iterator iter = componentsIter.first; iter != componentsIter.second; ++iter)
      {
         CGameObjectComponent * pComponent = (*iter).second;
         if (!bCopyProperties)
         {
            if (!pComponent->HasDuplicatedProperties())  // If this component has duplicated properties, then we didn't change them.
            {
               pComponent->PropertiesChanged();  // Hello! Your properties have changed.
            }
         }
         else
         {
            if (!pComponent->HasDuplicatedProperties())
            {
               pComponent->DuplicateProperties();
            }
            CGameObjectComponentProperties &componentProperties = *const_cast<CGameObjectComponentProperties*>(pComponent->GetProperties().get());
            ModifyComponentProperties(componentProperties, propertiesStream);
            pComponent->PropertiesChanged();  // Hello! Your properties have changed.
            // Can only modify one set of properties at the moment as we need to duplicate the stream
            break;
         }
      }

      // We've processed this property stream, discard it
      return true;
   }
   else
   {
      // See if it's a unique id
      CGameObjectComponent * pComponent = ComponentFromUniqueId(componentEditorOrUniqueId);
      if (pComponent)
      {
         if (!bCopyProperties)
         {
            // We shouldn't really have a case where we're modifing component properties with a unique id,
            // as this would only happen currently when using a property animator.
            if (!pComponent->HasDuplicatedProperties())  // If this component has duplicated properties, then we didn't change them.
            {
               pComponent->PropertiesChanged();  // Hello! Your properties have changed.
            }
         }
         else
         {
            if (!pComponent->HasDuplicatedProperties())
            {
               pComponent->DuplicateProperties();
            }
            CGameObjectComponentProperties &componentProperties = *const_cast<CGameObjectComponentProperties*>(pComponent->GetProperties().get());
            ModifyComponentProperties(componentProperties, propertiesStream);
            pComponent->PropertiesChanged();  // Hello! Your properties have changed.
         }
      }
   }

   // no match, leave stream and allow other groups of components to use these component property modifications.
   return false;
}

//----------------------------------------------------------------------------

void CBaseGameManager::ModifyComponentProperties(CGameObjectComponentProperties &componentProperties, CInputStream &propertiesStream) const
{
   // Evaluators/Components that have changed type will be ignored.
   // Evaluators/Components that have remained the same type will have their properties updated.
   CGameObjectApplyPropertiesData applyProperties(propertiesStream, mpGameObjectFactory->GetEvaluatorFactory(), CGameObjectApplyPropertiesData::kUpdateOnly);
   componentProperties.ApplyProperties(applyProperties, componentProperties.mEvaluatorAllocator);
   componentProperties.PostLoadUpdate();  // Reload resources if required
}

//----------------------------------------------------------------------------

CComponentEditorIdToPropertiesMapping const * const CBaseGameManager::GetComponentEditorIdToPropertiesMapping(TComponentEditorId const &editorId,
                                                                                                              TGameObjectUniqueId const &packageRootId) const
{
   TComponentEditorIdToPropertiesMapConstIterPair componentsIt = mComponentEditorIdToPropertiesMap.equal_range(editorId);
   for (TComponentEditorIdToPropertiesMap::const_iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
   {
      CComponentEditorIdToPropertiesMapping const & mapping = (*iter).second;
      if (mapping.mPackageRootId == packageRootId)
      {
         return &mapping;
      }
      else
      {
         // Try traversing up properties root to find match
         CGameObject const * pGameObjectPackageRoot = GetAllObjects().GetObjectById(packageRootId);
         while (pGameObjectPackageRoot)
         {
            if (pGameObjectPackageRoot->GetPackageRootId() == packageRootId)
            {
               return &mapping;
            }
            // Move to parent
            pGameObjectPackageRoot = GetAllObjects().GetObjectById(pGameObjectPackageRoot->GetPackageRootId());
         }
      }
   }

   // Can't find package root id match, just return first editor id match
   TComponentEditorIdToPropertiesMap::const_iterator iter = mComponentEditorIdToPropertiesMap.find(editorId);
   if (iter == mComponentEditorIdToPropertiesMap.end())
   {
      // no match
      return NULL;
   }

   CComponentEditorIdToPropertiesMapping const &mapping = (*iter).second;
   return &mapping;
}

//----------------------------------------------------------------------------

