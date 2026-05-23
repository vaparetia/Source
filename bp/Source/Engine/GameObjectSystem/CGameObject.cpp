//------------------------------------------------------------------------------------------
// CGameObject.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CGameObject.h"

//------------------------------------------------------------------------------------------

#include "Engine/Mechanics/CCRC.h"
#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "Engine/GameObjectSystem/CGameObjectComponentMethods.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"
#include "Engine/GameObjectSystem/CGameObjectMessageHandler.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CGameObject::CGameObject( TGameObjectUniqueId const &id )
:  mId( id )
,  mpParent( NULL )
,  mPackageRootId( kInvalidGameObjectUniqueId )
,  mInstanceTime( 0 )
,  mLastUpdateCount((uint32) -1)
,  mSharedFlags(0)
{
}

//------------------------------------------------------------------------------------------
CGameObject::CGameObject(  boost::shared_ptr<CGameObjectProperties const> pProperties, 
                           IGameObjectFactory &factory,
                           TGameObjectUniqueId const &id )
:  mId(id)
,  mpParent( NULL )
,  mPackageRootId( kInvalidGameObjectUniqueId )
,  mpProperties(pProperties)
,  mInstanceTime( 0 )
,  mLastUpdateCount((uint32) -1)
,  mSharedFlags(0)
{
   CGameObjectProperties::TComponentProperties const &componentPropertiesArray = pProperties->mComponentProperties;
   mpComponents.reserve(componentPropertiesArray.size());
   for (CGameObjectProperties::TComponentProperties::const_iterator iter = componentPropertiesArray.begin(); iter != componentPropertiesArray.end(); ++iter)
   {
      boost::shared_ptr<CGameObjectComponentProperties const> pComponentProperties = *iter;
      TComponent pComponent(factory.BuildComponent(pComponentProperties));
      mpComponents.push_back(pComponent);
   }
}
//------------------------------------------------------------------------------------------

CGameObject::~CGameObject()
{
   // We unparent in destructor so child lists don't get smaller during think.
   // Notify parent
   if (mpParent)
   {
      mpParent->RemoveChild(this);
   }
   foreach(CGameObject *pChild, mChildren)
   {
      // Remove this GO as a parent
      pChild->SetParent(NULL);
   }
   // Delete components
   foreach(CGameObjectComponent *pComponent, mpComponents)
   {
      delete pComponent;
   }
}

//------------------------------------------------------------------------------------------

void CGameObject::Initialize(CBaseGameManager &baseGameManager, TGameObjectUniqueId const &packageRootId, CGameObjectConstructionParams const &params)
{
   mPackageRootId = packageRootId;
   for (TComponents::iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent * const pComponent = (*iter);
      TComponentUniqueId const componentUniqueId(TComponentUniqueId::Create());
      baseGameManager.AddComponentUniqueIdToComponentMapping(componentUniqueId, pComponent);
      pComponent->Initialize(baseGameManager, *this, componentUniqueId, params);
   }

   // If you've got any methods to bind, call BindMethods from 'derived'::Initialized after calling this function.
}

//------------------------------------------------------------------------------------------

void CGameObject::Uninitialize(CBaseGameManager &baseGameManager)
{
   DeleteAllChildren(baseGameManager);      

   UninitializeComponents(baseGameManager);
   BPE_ASSERT(!Deleted(), "CGameObject::Uninitialize: Object already deleted!");
   SetSharedFlag(skSF_Deleted, 1);
}

//------------------------------------------------------------------------------------------

void CGameObject::UninitializeComponents(CBaseGameManager &baseGameManager)
{
   // Notify components
   for (TComponents::iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent * const pComponent = (*iter);
      baseGameManager.RemoveComponentUniqueIdToComponentMapping(pComponent->GetId());
      pComponent->Uninitialize(baseGameManager);
   }
}

//------------------------------------------------------------------------------------------

void CGameObject::BindMethods(CGameObjectComponentMethods &methods) const
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent const * const pComponent = (*iter);
      pComponent->BindMethods(methods);
   }
}

//------------------------------------------------------------------------------------------

CGameObjectComponentMethods &CGameObject::GetBindMethods()
{
   BPE_ASSERTA("No bind methods available, implement in derived class.");
   static CGameObjectComponentMethods sNullMethods;
   return sNullMethods;
}


//------------------------------------------------------------------------------------------

TGameObjectUniqueId const & CGameObject::GetParentId() const
{
   if (mpParent)
   {
      return mpParent->GetId();
   }
   return kInvalidGameObjectUniqueId;
}

//------------------------------------------------------------------------------------------

void CGameObject::AddChild( CGameObject *pGO )
{
   BPE_ASSERT(find( mChildren.begin(), mChildren.end(), pGO ) == mChildren.end(), "Already in child list!");
   BPE_ASSERT(pGO->GetParentId() != GetId(), "Parent and child are the same!");
   CGameObject * pOldParent = pGO->Parent();
   // Might not have a parent if new
   if (pOldParent)
   {
      //BPE_VERIFY( pOldParent != NULL, false, "object must exist to be added as child, maybe forgot to add to object system?" );
      pOldParent->RemoveChild( pGO );
   }

   pGO->SetParent( this );
   mChildren.push_back( pGO );
}

//------------------------------------------------------------------------------------------

void CGameObject::RemoveChild( CGameObject *pGO )
{
   TChildren::iterator found = find( mChildren.begin(), mChildren.end(), pGO );
   if( found != mChildren.end() )
   {
      mChildren.erase( found );

      if( pGO )
      {
         pGO->SetParent( NULL );
      }
   }
}

//------------------------------------------------------------------------------------------

void CGameObject::DeleteAllChildren(CBaseGameManager &baseGameManager)
{
   // Delete child objects
   for (int loop = 0; loop < mChildren.size(); loop++)
   {
      CGameObject * pChild = mChildren[loop];
      baseGameManager.DeleteObject(pChild->GetId());
   }
}

//------------------------------------------------------------------------------------------

void CGameObject::DeleteYourself(CBaseGameManager &baseGameManager)
{
   // Post a delete message, when processed, parent object will be deleted
   baseGameManager.MessageHandler().AddDeleteMessage(GetId());
   SetSharedFlag(skSF_PendingDeletion, 1);
}

//------------------------------------------------------------------------------------------

void CGameObject::SetParent( CGameObject *pGO )
{
   if( pGO )
      BPE_VERIFY( !mpParent, false, "can not have multiple parents, must remove from parent previous parent before adding as a child to new parent" );

   mpParent = pGO;
}

//------------------------------------------------------------------------------------------

CGameObject::TComponent CGameObject::ComponentByType(uint32 const groupType, uint32 const componentType)
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;

      if (  (pComponent->GetProperties()->GetComponentGroupType() == groupType) &&
            (pComponent->GetProperties()->GetComponentType() == componentType))
      {
         return pComponent;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject::TConstComponent CGameObject::GetComponentByType(uint32 const groupType, uint32 const componentType) const
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;

      if (  (pComponent->GetProperties()->GetComponentGroupType() == groupType) &&
            (pComponent->GetProperties()->GetComponentType() == componentType))
      {
         return pComponent;
      }
   }

   return NULL;
}
//------------------------------------------------------------------------------------------

CGameObject::TComponent CGameObject::BaseComponentByType(uint32 const groupType)
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;

      if (pComponent->GetProperties()->GetComponentGroupType() == groupType)
      {
         return pComponent;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject::TConstComponent CGameObject::GetBaseComponentByType(uint32 const groupType) const
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;
      if (pComponent->GetProperties()->GetComponentGroupType() == groupType)
      {
         return *iter;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject::TComponent CGameObject::ComponentByEditorId(TComponentEditorId const & componentEditorId)
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;
      if (pComponent->GetProperties()->mEditorId == componentEditorId)
      {
         return pComponent;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject::TConstComponent CGameObject::GetComponentByEditorId(TComponentEditorId const & componentEditorId) const
{
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;
      if (pComponent->GetProperties()->mEditorId == componentEditorId)
      {
         return pComponent;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------------------
// Used to find components for message remapping
// We might be able to speed this up later by using the editor id->component mapping in base game manager
CGameObject::TConstComponent CGameObject::GetComponentByEditorIdIncludingChildren(CBaseGameManager const &manager, TComponentEditorId const & componentEditorId) const
{
   TConstComponent pComponent = GetComponentByEditorId(componentEditorId);
   if (pComponent != NULL) return pComponent;

   // Nope, see if any of it's children have got this component
   for (TChildren::const_iterator it = mChildren.begin(); it != mChildren.end(); ++it)
   {
      CGameObject const * const pChild = *it;
      BPE_ASSERT(pChild != NULL, "Missing child!");
      TConstComponent pChildComponent = pChild->GetComponentByEditorIdIncludingChildren(manager, componentEditorId);
      if (pChildComponent != NULL) return pChildComponent;
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

void CGameObject::RemapMessagesAndLinksToUniqueIds(CBaseGameManager &manager, CGameObject const &remapParent, CGameObject const * const pRemapToGenerator, int32 const * const pExcludeMessageType)
{
   // Components
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;
      pComponent->RemapMessagesAndLinksToUniqueIds(manager, remapParent, pRemapToGenerator, pExcludeMessageType);
   }

   // Children
   for (TChildren::const_iterator it = mChildren.begin(); it != mChildren.end(); ++it)
   {
      CGameObject * const pChild = *it;
      BPE_ASSERT(pChild != NULL, "Missing child!");
      pChild->RemapMessagesAndLinksToUniqueIds(manager, remapParent, pRemapToGenerator, pExcludeMessageType);
   }
}

//------------------------------------------------------------------------------------------

void CGameObject::PostLoadInitializeRecursive(CBaseGameManager &manager)
{
   // Call children first then parent, seems like parent needs it children ready first
   // Children
   for (TChildren::const_iterator it = mChildren.begin(); it != mChildren.end(); ++it)
   {
      CGameObject * const pChild = *it;
      BPE_ASSERT(pChild != NULL, "Missing child!");
      pChild->PostLoadInitializeRecursive(manager);
   }

   // Components, don't call if already been initialized (can occur with immediate send generate or package load)
   if (!PostLoadInitialized())
   {
      for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
      {
         CGameObjectComponent *pComponent = *iter;
         pComponent->PostLoadInitialize(manager);
      }
   }
   SetSharedFlag(skSF_PostLoadInitialized, 1);
}

//------------------------------------------------------------------------------------------

void CGameObject::SetIdentifier(std::string const &identifier)
{
   mIdentifierString = identifier;
}

//------------------------------------------------------------------------------------------

std::string const & CGameObject::GetEditorName() const
{
   return mpProperties->mEditorName;
}

//------------------------------------------------------------------------------------------

#include "Engine/Mechanics/TinyXml/tinyxml.h"

TiXmlNode * CGameObject::LogInfo(CBaseGameManager const &manager, TiXmlNode * pXml) const
{
   TiXmlElement * pInfo = new TiXmlElement("GameObject");
   pInfo->SetAttribute("name", mpProperties->mEditorName);
   if (mIdentifierString.size())
   {
      pInfo->SetAttribute("identifier", mIdentifierString);
   }
   // Comment out long strings until we have better game connection output
   //pInfo->SetAttribute("editorId", mpProperties->mEditorId.AsString());
   //pInfo->SetAttribute("id", mId.AsString());

   // Components
   for (TComponents::const_iterator iter = mpComponents.begin(); iter != mpComponents.end(); ++iter)
   {
      CGameObjectComponent *pComponent = *iter;
      
      pComponent->LogInfo(manager, pInfo);
   }

   // Children
   for (TChildren::const_iterator it = mChildren.begin(); it != mChildren.end(); ++it)
   {
      CGameObject const * const pChild = *it;
      BPE_ASSERT(pChild != NULL, "Missing child!");
      pChild->LogInfo(manager, pInfo);
   }

   if (pXml)
   {
      pXml->LinkEndChild(pInfo);
   }

   return pInfo;
}

//------------------------------------------------------------------------------------------
