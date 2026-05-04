//------------------------------------------------------------------------------------------
// CGameObjectMessageHandler.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/GameObjectSystem/CGameObjectMessageHandler.h"
#include "Engine/GameObjectSystem/CGameObjectMessageProperties.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"
#include "Engine/Mechanics/CCRC.h"
#include "Engine/System/CSyncCriticalSection.h"

//------------------------------------------------------------------------------------------
static uint32 skMessage_Delete                          = 0xDEADDEAD;
//------------------------------------------------------------------------------------------

CGameObjectMessageHandlerEntry::CGameObjectMessageHandlerEntry(CGameObjectMessage const &message,
                                                               real64 const triggerTime, 
                                                               TComponentUniqueId const &senderId,
                                                               TComponentUniqueId const &originatorId,
                                                               std::string const &additionalDestQuery)

: mMessage(message)
, mTriggerTime(triggerTime)
, mSenderId(senderId)
, mOriginatorId(originatorId)
, mAdditionalDestQuery(additionalDestQuery)
{
}

//------------------------------------------------------------------------------------------

CGameObjectMessageHandler::CGameObjectMessageHandler(CBaseGameManager &manager)
:  mGameManager(manager)
,  mCurrentlySendingMessages(false)
,  mpDeleteMessage(new CGameObjectMessagePropertiesEmpty(skMessage_Delete))
{
}

//------------------------------------------------------------------------------------------

CGameObjectMessageHandler::~CGameObjectMessageHandler()
{
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::AddDeleteMessage(TGameObjectUniqueId const &destId)
{
   // Delete messages are special, and the id refers to a game object, not a component.
   CGameObjectMessage const deleteMessage(0xFFFFFFFF, TComponentId::FromGuid(destId), mpDeleteMessage);
   AddMessage(deleteMessage, kInvalidComponentUniqueId, kInvalidComponentUniqueId, std::string());
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::AddMessage(CGameObjectMessage const &message, TComponentUniqueId const &senderComponentUniqueId, TComponentUniqueId const &originatorId, std::string const &additionalDestQuery)
{
   // Check for immediate send message
   int32 const skSendImmediatelyPriority = 999;
   // Can't sent delete messages immediately as this will screw up object deletion.
   if ((message.mPriority == skSendImmediatelyPriority) && (message.mpProperties->GetMessageType() != skMessage_Delete))
   {
      // Special case for messages that must be sent immediately, no time delay or priority.
      // Useful for speedups.
      SendMessageToComponent(message.mDestId, message, senderComponentUniqueId, originatorId, additionalDestQuery);
      return;
   }

   real64 const triggerTime = message.mTriggerTime + mGameManager.GetUpdateTime();
   if (mCurrentlySendingMessages)
   {
      // We're currently sending messages, so we don't want to invalidate iterators whilst doing this
      // We'll cache these messages and they will get resent after we've finished the current batch
      mPendingMessagesCache.insert(TMessageMap::value_type(CGameObjectMessageHandlerEntryKey(triggerTime, message.mPriority),
                                                           CGameObjectMessageHandlerEntry(message, triggerTime, senderComponentUniqueId, originatorId, additionalDestQuery)));
   }
   else
   {
      mPendingMessages.insert(TMessageMap::value_type(CGameObjectMessageHandlerEntryKey(triggerTime, message.mPriority),
                                                      CGameObjectMessageHandlerEntry(message, triggerTime, senderComponentUniqueId, originatorId, additionalDestQuery)));
   }
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::SendMessages(real64 const startTime, real64 const endTime, CSyncCriticalSection * pCS)
{
   mCurrentlySendingMessages = true;
   while (true)
   {
      TMessageMap::iterator startIt = mPendingMessages.lower_bound(CGameObjectMessageHandlerEntryKey(startTime, gkInt32Max));
      real64 const kEndTimeFudge = 1.0E-4;   // Add 1/10ms to make sure that messages sent on current frame get processed when time step is zero.
      TMessageMap::iterator endIt = mPendingMessages.upper_bound(CGameObjectMessageHandlerEntryKey(endTime + kEndTimeFudge, gkInt32Max));
      if (startIt != endIt)
      {
         // Only lock if there are messages to send.
         // Use platform specific version as PS3 doesn't need to lock.
         BPE_CRITCAL_SECTION_LOCK_MT_PS(pCS);

         for (TMessageMap::iterator it = startIt; it != endIt; ++it)
         {
            CGameObjectMessageHandlerEntry const &entry = (*it).second;
            //bpe_debugger_printf("%0.4f, 0x%08x/0x%08x\n", entry.mTriggerTime, entry.mMessage.mEventId, entry.mMessage.mpProperties->GetMessageType());
            if (entry.mMessage.mpProperties->GetMessageType() == skMessage_Delete)
            {
               // Delete messages are special, and the id refers to a game object, not a component.
               TGameObjectUniqueId const destId = TGameObjectId::FromGuid(entry.mMessage.mDestId);

               // Special delete message, remove object
               mGameManager.DeleteObject(destId);
            }
            else
            {
               SendMessageToComponent(entry.mMessage.mDestId, entry.mMessage, entry.mSenderId, entry.mOriginatorId, entry.mAdditionalDestQuery);
            }
         }
         // Delete sent messages
         mPendingMessages.erase(startIt, endIt);
      }
      
      if (mPendingMessagesCache.size())
      {
         // Reinsert new messages received
         mPendingMessages.insert(mPendingMessagesCache.begin(), mPendingMessagesCache.end());
         mPendingMessagesCache.clear();
      }
      else
      {
         // No more messages to send
         break;
      }
   }     

   mCurrentlySendingMessages = false;
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::SendMessageToComponent(  TComponentId const &destId, 
                                                         CGameObjectMessage const &message, 
                                                         TComponentUniqueId const &senderComponentUniqueId,
                                                         TComponentUniqueId const &originatorId,
                                                         std::string const &additionalDestQuery) const
{
   // Perform callbacks
   uint32 callbackResult = kNone;
   foreach(TMessageCallback const &callback, mMessageCallbacks)
   {
      callbackResult |= callback(destId, message, senderComponentUniqueId, originatorId);
      if ((callbackResult & kSkipAllOtherCallbacks) != 0)
      {
         break;
      }
   }
   if ((callbackResult & kSkipInternalSend) != 0) return;

   // Lets see if we can find a component with this as an unique id (messages can get a dest unique id after being remapped during object generation).
   // We generally have more unique ids than editor ids.
   if (destId.IsUniqueId())
   {
      CGameObjectComponent * const pComponent = mGameManager.ComponentFromUniqueId(destId);
      if (pComponent)
      {
         SendMessageToComponent(pComponent, message, senderComponentUniqueId, originatorId, additionalDestQuery);
      }
      return;
   }

   // It's an editor id
   // Check for components with this editor id
   CBaseGameManager::TComponentEditorIdToComponentMap const &map = mGameManager.GetComponentEditorIdToComponentMap();
   
   // Send this message to all components with this editor id
   for (CBaseGameManager::TComponentEditorIdToComponentMap::const_iterator iter = map.find(destId); iter != map.end() && ((*iter).first == destId); ++iter)
   {
      CGameObjectComponent * pComponent = (*iter).second;
      BPE_ASSERT(pComponent->GetEditorId() == destId, "EditorId to UniqueId mapping inconsistent!");
      SendMessageToComponent(pComponent, message, senderComponentUniqueId, originatorId, additionalDestQuery);
   }
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::SendMessageToComponent(  CGameObjectComponent *pComponent,
                                                         CGameObjectMessage const &message, 
                                                         TComponentUniqueId const &senderComponentUniqueId,
                                                         TComponentUniqueId const &originatorId,
                                                         std::string const &additionalDestQuery) const
{
   if (pComponent->GetGameObject()->Deleted())
   {
      // Don't sent messages to deleted components
      return;
   }

   // Src queries occur when the message is originally sent.
   bool bPassedQuery = true;
   if (message.mDestQuery.size() != 0)
   {
      bPassedQuery = pComponent->RunQuery(message.mDestQuery);
   }
   if (bPassedQuery && additionalDestQuery.size() != 0)
   {
      bPassedQuery = pComponent->RunQuery(additionalDestQuery);
   }

   if (!bPassedQuery) return;

   // Send message to component
   // Check common handler first
   CGameObjectMessageInfo const info(senderComponentUniqueId, originatorId);
   pComponent->InternalProcessMessage(mGameManager, message, info);
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::AddMessageCallback(TMessageCallback const &callback)
{
   if (!HasMessageCallback(callback))
   {
      mMessageCallbacks.push_back(callback);
   }
}

//------------------------------------------------------------------------------------------

bool CGameObjectMessageHandler::HasMessageCallback(TMessageCallback const &callback) const
{
   bool const bFound = std::find(mMessageCallbacks.begin(), mMessageCallbacks.end(), callback) != mMessageCallbacks.end();
   return bFound;
}

//------------------------------------------------------------------------------------------

void CGameObjectMessageHandler::RemoveMessageCallback(TMessageCallback const &callback)
{
   for(bpe::reserved_vector<TMessageCallback, 4>::iterator it = mMessageCallbacks.begin(); it != mMessageCallbacks.end(); ++it)
   {
      if ((*it) == callback)
      {
         mMessageCallbacks.erase(it);
         break;
      }
   }
}

//------------------------------------------------------------------------------------------


