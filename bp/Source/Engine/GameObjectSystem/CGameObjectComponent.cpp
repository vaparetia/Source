//------------------------------------------------------------------------------------------
// CGameObjectComponent.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CGameObjectComponent.h"
#include "CGameObjectComponentProperties.h"
#include "CGameObjectMessageProperties.h"
#include "CGameObjectMessage.h"
#include "CGameObjectMessageHandler.h"
#include "GameObjectUtils.h"
#include "Engine/Mechanics/CCRC.h"

//------------------------------------------------------------------------------------------

#include "CBaseGameManager.h"
#include "CGameObjectComponentMethods.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------
// Cannot pass shared_ptr by reference if you want to cast
CGameObjectComponent::CGameObjectComponent( boost::shared_ptr<CGameObjectComponentProperties const > pProperties )
:  mpProperties(pProperties)
,  mpOriginalProperties(pProperties)
,  mId(kInvalidComponentUniqueId)
,  mpGameObjectOwner(NULL)
{
}

//------------------------------------------------------------------------------------------

CGameObjectComponent::~CGameObjectComponent()
{
}

//------------------------------------------------------------------------------------------

TComponentEditorId const & CGameObjectComponent::GetEditorId() const
{
   return mpProperties->mEditorId;
}

//------------------------------------------------------------------------------------------

TGameObjectUniqueId const & CGameObjectComponent::GetGameObjectId() const
{
   return mpGameObjectOwner->GetId();
}

//------------------------------------------------------------------------------------------

TGameObjectEditorId const & CGameObjectComponent::GetGameObjectEditorId() const
{
   return mpGameObjectOwner->GetProperties()->mEditorId;
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::Initialize(CBaseGameManager &baseGameManager, CGameObject &gameObjectOwner, TComponentUniqueId const &componentUniqueId, CGameObjectConstructionParams const &params)
{
   (baseGameManager);
   (params);
   mId = componentUniqueId;
   mpGameObjectOwner = &gameObjectOwner;
}


//------------------------------------------------------------------------------------------

void CGameObjectComponent::PostLoadInitialize(CBaseGameManager &baseGameManager)
{
   (baseGameManager);
}

//------------------------------------------------------------------------------------------
// Called before destruction
void CGameObjectComponent::Uninitialize(CBaseGameManager &baseGameManager)
{
   (baseGameManager);
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::BindMethods(CGameObjectComponentMethods &methods) const
{
   (methods);
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::PropertiesChanged()
{
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::InternalProcessMessage(CBaseGameManager &, CGameObjectMessage const &, CGameObjectMessageInfo const &)
{
}

//------------------------------------------------------------------------------------------

bool CGameObjectComponent::RunQuery(std::string const &queryString) const
{
   // Deliberately empty.
   // CGameObjectComponentGame uses LUA scripting for queries, and doesn't call into this function.

   return true;
}

//------------------------------------------------------------------------------------------

bool CGameObjectComponent::HasEvent(int32 const eventId) const
{
   // Use local messages if any
   TMessages const * pMessages = &mpProperties->mMessages;
   if (mMessages.size())
   {
      pMessages = &mMessages;
   }
      
   for (TMessages::const_iterator it = pMessages->begin(); it != pMessages->end(); ++it)
   {
      CGameObjectMessage const &message = *it;
      if (message.mEventId == eventId)
      {
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::SendEventMessages(CBaseGameManager &manager, int32 const eventId, TComponentUniqueId const * const pOriginator, std::string const * const pAdditionalDestQuery) const
{
   // Use local messages if any
   TMessages const * pMessages = &mpProperties->mMessages;
   if (mMessages.size())
   {
      pMessages = &mMessages;
   }

   CGameObjectMessageHandler &messageHandler = manager.MessageHandler();
   for (TMessages::const_iterator it = pMessages->begin(); it != pMessages->end(); ++it)
   {
      CGameObjectMessage const &message = *it;
      if (message.mEventId == eventId)
      {
         SendSingleMessage(manager, message, pOriginator, pAdditionalDestQuery);
      }
   }
}

//----------------------------------------------------------------------------

void CGameObjectComponent::SendSingleMessage(CBaseGameManager &manager, CGameObjectMessage const &message, TComponentUniqueId const * const pOriginator, std::string const * const pAdditionalDestQuery) const
{
   bool bPassedQuery = true;
   if (message.mSenderQuery.size() != 0)
   {
      // Test the query current component.
      // Destination queries occur when message is received.
       bPassedQuery = RunQuery(message.mSenderQuery);
   }

   // Only send if passed sender query
   if (bPassedQuery)
   {
      CGameObjectMessageHandler &messageHandler = manager.MessageHandler();
      messageHandler.AddMessage(message, mId, pOriginator ?  *pOriginator : mId, pAdditionalDestQuery ? *pAdditionalDestQuery : CStringExtras::skEmptyString);
   }
}

//----------------------------------------------------------------------------

void CGameObjectComponent::SendSimpleMessage(CBaseGameManager &manager, 
                                             int32 const message, 
                                             TComponentUniqueId const &destId, 
                                             TComponentUniqueId const * const pOriginatorId, 
                                             real32 const time, 
                                             int32 const priority, 
                                             std::string const &senderQuery,
                                             std::string const &destQuery,
                                             std::string const &additionalDestQuery) const

{
   // We'll probably want to find a method where we don't allocate later
   CGameObjectMessagePropertiesEmpty *pSimpleMsg = new CGameObjectMessagePropertiesEmpty((uint32) message);
   // Don't care about the event as it's generally only used before the message is sent.
   int32 const kNoEvent = 0;
   manager.MessageHandler().AddMessage(CGameObjectMessage(  kNoEvent, 
                                                            destId, 
                                                            boost::shared_ptr<CGameObjectMessageProperties const>(pSimpleMsg),
                                                            time, 
                                                            priority, 
                                                            senderQuery, 
                                                            destQuery),
                                                            GetId(), pOriginatorId ? *pOriginatorId : GetId(), additionalDestQuery);
}

//----------------------------------------------------------------------------

TComponentId const & CGameObjectComponent::GetFirstLinkedComponentId(int32 const linkId) const
{
   // Use local messages if any
   TMessages const * pLinks = &mpProperties->mLinks;
   if (mLinks.size())
   {
      pLinks = &mLinks;
   }

   for (TMessages::const_iterator it = pLinks->begin(); it != pLinks->end(); ++it)
   {
      CGameObjectMessage const &link = *it;
      if (link.mEventId == linkId)
      {
         // This could be an editor id or unique id
         return link.mDestId;
      }
   }

   return kInvalidComponentId;
}

//----------------------------------------------------------------------------

CGameObjectComponent const * CGameObjectComponent::GetFirstLinkedComponent(int32 const linkId, CBaseGameManager const &manager, std::string const &destQuery) const
{
   return const_cast<CGameObjectComponent*>(this)->FirstLinkedComponent(linkId, const_cast<CBaseGameManager&>(manager), destQuery);
}

//----------------------------------------------------------------------------

CGameObjectComponent * CGameObjectComponent::FirstLinkedComponent(int32 const linkId, CBaseGameManager &manager, std::string const &destQuery)
{
   // Use local messages if any
   TMessages const * pLinks = &mpProperties->mLinks;
   if (mLinks.size())
   {
      pLinks = &mLinks;
   }

   for (TMessages::const_iterator it = pLinks->begin(); it != pLinks->end(); ++it)
   {
      CGameObjectMessage const &link = *it;
      if (link.mEventId == linkId)
      {
         // Return the first match
         CGameObjectComponent * pComponent = link.mDestId.FirstComponentFromUniqueOrEditorId(manager, destQuery);
         
         if( pComponent)
         {
            return pComponent;
         }
      }
   }

   return NULL;
}

//----------------------------------------------------------------------------

CGameObjectComponent const * CGameObjectComponent::GetFirstLinkedUniqueIdComponent(int32 const linkId, CBaseGameManager const &manager, std::string const &destQuery) const
{
   return const_cast<CGameObjectComponent*>(this)->FirstLinkedUniqueIdComponent(linkId, const_cast<CBaseGameManager&>(manager), destQuery);
}

//----------------------------------------------------------------------------

CGameObjectComponent * CGameObjectComponent::FirstLinkedUniqueIdComponent(int32 const linkId, CBaseGameManager &manager, std::string const &destQuery)
{
   // Use local messages if any
   TMessages const * pLinks = &mpProperties->mLinks;
   if (mLinks.size())
   {
      pLinks = &mLinks;
   }

   for (TMessages::const_iterator it = pLinks->begin(); it != pLinks->end(); ++it)
   {
      CGameObjectMessage const &link = *it;
      if (link.mEventId == linkId)
      {
         // Return the first match from a unique id
         CGameObjectComponent * pComponent = static_cast<TComponentUniqueId>(link.mDestId).Component(manager);
         if (pComponent)
         {
            return pComponent;
         }
      }
   }

   return NULL;
}
//----------------------------------------------------------------------------

int CGameObjectComponent::GetLinkedComponents(CBaseGameManager const &manager, int32 const linkId, TConstComponentListTransient &linkedComponents) const
{
   int count = 0;

   // Use local messages if any
   TMessages const * pLinks = &mpProperties->mLinks;
   if (mLinks.size())
   {
      pLinks = &mLinks;
   }

   for (TMessages::const_iterator it = pLinks->begin(); it != pLinks->end(); ++it)
   {
      CGameObjectMessage const &link = *it;
      if (link.mEventId == linkId)
      {
         count += manager.GetComponentsFromUniqueOrEditorId(link.mDestId, linkedComponents);
      }
   }

   return count;
}

//----------------------------------------------------------------------------

int CGameObjectComponent::LinkedComponents(CBaseGameManager &manager, int32 const linkId, TComponentListTransient &linkedComponents) const
{
   int count = 0;

   // Use local messages if any
   TMessages const * pLinks = &mpProperties->mLinks;
   if (mLinks.size())
   {
      pLinks = &mLinks;
   }

   for (TMessages::const_iterator it = pLinks->begin(); it != pLinks->end(); ++it)
   {
      CGameObjectMessage const &link = *it;
      if (link.mEventId == linkId)
      {
         count += manager.ComponentsFromUniqueOrEditorId(link.mDestId, linkedComponents);
      }
   }

   return count;
}

//----------------------------------------------------------------------------

CGameObjectComponent::TMessages const & CGameObjectComponent::GetMessages() const
{
   if (mMessages.size()) return mMessages;
   return mpProperties->mMessages;
}

//----------------------------------------------------------------------------

CGameObjectComponent::TMessages const & CGameObjectComponent::GetLinks() const
{
   if (mLinks.size()) return mLinks;
   return mpProperties->mLinks;
}

//----------------------------------------------------------------------------

void CGameObjectComponent::DeleteYourself(CBaseGameManager &manager)
{
   GameObject()->DeleteYourself(manager);
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::DuplicateProperties()
{
   mpProperties.reset(mpOriginalProperties->clone());
}

//------------------------------------------------------------------------------------------

bool CGameObjectComponent::HasDuplicatedProperties() const
{
   return (mpProperties.get() != mpOriginalProperties.get());
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::DuplicateMessagesAndLinks()
{
   // Duplicate messages and links, but only if they haven't already been duplicated.
   if (mMessages.size() == 0) mMessages = mpProperties->mMessages;
   if (mLinks.size() == 0) mLinks = mpProperties->mLinks;
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::RemapMessagesAndLinksToUniqueIds(CBaseGameManager &manager, CGameObject const &remapParent, CGameObject const * const pRemapToGenerator, int32 const * const pExcludeMessageType)
{
   // Duplicate messages and links, but only if they haven't already been duplicated.
   // This is because this function might be called multiple times for remapping between 
   // siblings and we want to keep the earlier remapping results.
   DuplicateMessagesAndLinks();

   // Now convert component editor ids to component unique ids where the editor id exists with the remap parent
   for (TMessages::iterator it = mMessages.begin(); it != mMessages.end(); ++it)
   {
      CGameObjectMessage &message = *it;

      if (pExcludeMessageType && (message.mpProperties->GetMessageType() == *pExcludeMessageType))
      {
         // Don't remap this event, we want to keep pointing at original object
         continue;
      }
      
      // If this entry has already been remapped, then mDestId will be a component unique id, not editor id.
      // In this case, it will return a null component target, which is fine.
      CGameObject::TConstComponent pComponentTarget = remapParent.GetComponentByEditorIdIncludingChildren(manager, message.mDestEditorId);
      if (pComponentTarget != NULL)
      {
         // Dest component is within the remap parent tree, use unique id instead of editor id
         // This causes sent messages to go to only the component with the tree instead of all components instances using that editor id
         message.mDestId = pComponentTarget->GetId();
      }
      else if (pRemapToGenerator)
      {
         // See if we can remap this to the generator
         CGameObject::TConstComponent pComponentTarget = pRemapToGenerator->GetComponentByEditorId(message.mDestEditorId);
         if (pComponentTarget != NULL)
         {
            // Dest component is within generator object, use unique id instead of editor id
            // This causes sent messages to go to only the generator component instead of all components instances using that editor id
            message.mDestId = pComponentTarget->GetId();
         }
      }
   }

   for (TMessages::iterator it = mLinks.begin(); it != mLinks.end(); ++it)
   {
      CGameObjectMessage &link = *it;

      if (pExcludeMessageType && (link.mpProperties->GetMessageType() == *pExcludeMessageType))
      {
         // Don't remap this link, we want to keep pointing at original object         
         continue;
      }
      
      CGameObject::TConstComponent pComponentTarget = remapParent.GetComponentByEditorIdIncludingChildren(manager, link.mDestEditorId);
      if (pComponentTarget != NULL)
      {
         // Dest component is within the remap parent tree, use unique id instead of editor id
         // This causes sent messages to go to only the component with the tree instead of all components instances using that editor id
         link.mDestId = pComponentTarget->GetId();
      }
      else if (pRemapToGenerator)
      {
         // See if we can remap this to the generator
         CGameObject::TConstComponent pComponentTarget = pRemapToGenerator->GetComponentByEditorId(link.mDestEditorId);
         if (pComponentTarget != NULL)
         {
            // Dest component is within generator object, use unique id instead of editor id
            // This causes sent messages to go to only the generator component instead of all components instances using that editor id
            link.mDestId = pComponentTarget->GetId();
         }
      }
   }
}

//------------------------------------------------------------------------------------------

void CGameObjectComponent::AllocateEvaluatorDataBlock()
{
   if (!mpEvaluatorDataBlock)
   {
      if (mpProperties->mEvaluatorAllocator.mAllocSize)
      {
         mpEvaluatorDataBlock.reset( (uint8*) malloc(mpProperties->mEvaluatorAllocator.mAllocSize) );
      }
      else
      {
         // Not much point doing an allocation if zero size
         mpEvaluatorDataBlock.reset();
      }
   }
}

//------------------------------------------------------------------------------------------

CEvaluatorUpdateData CGameObjectComponent::GetInitializationEvaluatorUpdateData(CBaseGameManager &manager)
{
   AllocateEvaluatorDataBlock();

   CEvaluatorUpdateData updateData(&manager.Random(),
                                   CEvaluatorDataBlock(mpEvaluatorDataBlock.get()), 
                                   manager.GetUpdateTime(), 
                                   0.0f, 
                                   0.0f,
                                   &manager,
                                   this);
   return updateData;   
}
//------------------------------------------------------------------------------------------

CEvaluatorUpdateData CGameObjectComponent::GetEvaluatorUpdateData(CBaseGameManager &manager)
{
   CEvaluatorUpdateData updateData(&manager.Random(),
                                   CEvaluatorDataBlock(mpEvaluatorDataBlock.get()), 
                                   manager.GetUpdateTime(), 
                                   mpGameObjectOwner->GetInstanceTime(), 
                                   mpGameObjectOwner->GetInstanceTime(),     // We'll assume local time is same as instance time in this simple case
                                   &manager,
                                   this);
   return updateData;
}

//------------------------------------------------------------------------------------------

CManagedEvaluatorUpdateData CGameObjectComponent::GetTransientEvaluatorUpdateData(CBaseGameManager &manager, CEvaluatorAllocator const &allocator) const
{
   void *pDataBlockMemory = NULL;
   if (allocator.mAllocSize)
   {
      pDataBlockMemory = malloc(allocator.mAllocSize);
   }

   CEvaluatorUpdateData updateData(&manager.Random(),
                                   CEvaluatorDataBlock(pDataBlockMemory), 
                                   manager.GetUpdateTime(), 
                                   mpGameObjectOwner->GetInstanceTime(), 
                                   mpGameObjectOwner->GetInstanceTime(),
                                   &manager,
                                   this);  // We'll assume local time is same as instance time in this simple case

   return CManagedEvaluatorUpdateData(updateData);
}

//------------------------------------------------------------------------------------------

std::string const & CGameObjectComponent::GetEditorName() const
{
   return mpGameObjectOwner->GetEditorName();
}

//------------------------------------------------------------------------------------------

#include "Engine/Mechanics/TinyXml/tinyxml.h"

TiXmlNode * CGameObjectComponent::LogInfo(CBaseGameManager const &manager, TiXmlNode * pXml) const
{
   TiXmlElement * pInfo = new TiXmlElement("Component");

   pInfo->SetAttribute("groupName", mpProperties->GetComponentGroupName());
   pInfo->SetAttribute("typeName", mpProperties->GetComponentTypeName());

   // Comment out long strings until we have better game connection output
   //pInfo->SetAttribute("groupType", mpProperties->GetComponentGroupType());
   //pInfo->SetAttribute("type", mpProperties->GetComponentType());
   //pInfo->SetAttribute("editorId", mpProperties->mEditorId.AsString());
   //pInfo->SetAttribute("id", mId.AsString());

   pXml->LinkEndChild(pInfo);

   return pInfo;
}

//------------------------------------------------------------------------------------------

char const * CGameObjectComponent::GetMessageTypeName(int32 const messageType) const
{
   return "Unknown";
}

//------------------------------------------------------------------------------------------

char const * CGameObjectComponent::GetEventTypeName(int32 const messageType) const
{
   return "Unknown";
}

//------------------------------------------------------------------------------------------
