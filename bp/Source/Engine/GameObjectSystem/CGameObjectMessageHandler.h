//------------------------------------------------------------------------------------------
// CGameObjectMessageHandler.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "TGameObjectTypes.h"
#include "boost/shared_ptr.hpp"
#include "Engine/Mechanics/FastDelegate/FastDelegate.h"
#include "Engine/GameObjectSystem/CGameObjectMessage.h"
//------------------------------------------------------------------------------------------

class CBaseGameManager;
class CSyncCriticalSection;

//------------------------------------------------------------------------------------------
class ENGINE_API CGameObjectMessageHandlerEntry
{
public:
   CGameObjectMessageHandlerEntry(  CGameObjectMessage const &message,
                                    real64 const triggerTime, 
                                    TComponentUniqueId const &senderId,
                                    TComponentUniqueId const &originatorId,
                                    std::string const &additionalDestQuery);

   //bool operator < (CGameObjectMessageHandlerEntry const & rhs) const   { return mTriggerTime < rhs.mTriggerTime; };

   CGameObjectMessage         mMessage;
   real64                     mTriggerTime;  // CBaseGameManager time
   TComponentUniqueId         mSenderId;
   TComponentUniqueId         mOriginatorId;
   std::string                mAdditionalDestQuery;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectMessageHandlerEntryKey
{
public:
   CGameObjectMessageHandlerEntryKey(real64 const triggerTime, int32 const priority)
      : mTriggerTime(triggerTime)
      , mPriority(priority)
   {
   };

   bool operator < (CGameObjectMessageHandlerEntryKey const & rhs) const
   {
      if (mTriggerTime < rhs.mTriggerTime)
      {
         return true;
      }
      else if (mTriggerTime == rhs.mTriggerTime)
      {
         // If trigger times are the same then sort by priority
         return (mPriority > rhs.mPriority);
      }
      return false;
   }

   real64      mTriggerTime;
   int32       mPriority;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectMessageHandler
{
public:
   CGameObjectMessageHandler(CBaseGameManager &manager);
   ~CGameObjectMessageHandler();

   void     AddMessage(CGameObjectMessage const &message, TComponentUniqueId const &senderComponentUniqueId, TComponentUniqueId const &originatorId, std::string const &additionalDestQuery);
   void     AddDeleteMessage(TGameObjectUniqueId const &destId);   // Special message that deletes objects
   void     SendMessages(real64 const startTime, real64 const endTime, CSyncCriticalSection * pCS = NULL);
   void     SendMessageToComponent( TComponentId const &destId, 
                                    CGameObjectMessage const &message, 
                                    TComponentUniqueId const &senderComponentUniqueId,
                                    TComponentUniqueId const &originatorId,
                                    std::string const &additionalDestQuery) const;

   void     SendMessageToComponent(  CGameObjectComponent *pComponent,
                                     CGameObjectMessage const &message, 
                                     TComponentUniqueId const &senderComponentUniqueId,
                                     TComponentUniqueId const &originatorId,
                                     std::string const &additionalDestQuery) const;


   enum EMessageCallbackResult // Bit flags, return these in message callbacks
   {
      kNone                      = 0,
      kSkipInternalSend          = 1 << 0,
      kSkipAllOtherCallbacks     = 1 << 1,

      kTerminator                = 0xFFFFFFFF
   };   

   // Callbacks - destId, message, senderId, originatorId, EMessageCallbackResult
   typedef fastdelegate::FastDelegate4<TComponentId const &, CGameObjectMessage const &, TComponentUniqueId const &, TComponentUniqueId const &, uint32> TMessageCallback;

   void  AddMessageCallback(TMessageCallback const &callback);
   bool  HasMessageCallback(TMessageCallback const &callback) const;
   void  RemoveMessageCallback(TMessageCallback const &callback);

private:
   typedef std::multimap<CGameObjectMessageHandlerEntryKey, CGameObjectMessageHandlerEntry>     TMessageMap;
   typedef std::pair<TMessageMap::iterator, TMessageMap::iterator>   TMessageMapRangeIter;

   CBaseGameManager &   mGameManager;
   TMessageMap          mPendingMessages;
   bool                 mCurrentlySendingMessages;
   TMessageMap          mPendingMessagesCache;

   boost::shared_ptr<CGameObjectMessageProperties const> 
                        mpDeleteMessage;     // Cached here so we don't have to allocate every time we use this

   // Add slots to these to do custom processing on messages (e.g. generate)
   bpe::reserved_vector<TMessageCallback, 4>    mMessageCallbacks;

private:
   BPE_DISABLE_OBJECT_ASSIGN(CGameObjectMessageHandler);
};

//------------------------------------------------------------------------------------------

