//------------------------------------------------------------------------------------------
// CGameObjectMessage.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "TGameObjectTypes.h"
#include "boost/shared_ptr.hpp"
//------------------------------------------------------------------------------------------

typedef int32 TScriptEventId;

class CGameObjectMessageProperties;
class IGameObjectFactory;

//------------------------------------------------------------------------------------------
class ENGINE_API CGameObjectMessage
{
public:
   CGameObjectMessage(  TScriptEventId const eventId, 
                        TComponentId const &destId, 
                        boost::shared_ptr<CGameObjectMessageProperties const> const &pProperties,
                        real32 const triggerTime = 0.0f, 
                        int32 const priority = 0,
                        std::string const &senderQuery = std::string(),
                        std::string const &destQuery = std::string());
   ~CGameObjectMessage();

   static CGameObjectMessage const FMessageFactory(CInputStream &inStream, IGameObjectFactory &factory);

   TScriptEventId                                           mEventId;
   TComponentId                                             mDestId;                   // If not remapped in component, then component editor id, else can be component editor id or component unique id
   TComponentEditorId                                       mDestEditorId;             // Original editor id, used for remapping to unique id.
   real32                                                   mTriggerTime; 
   int32                                                    mPriority;
   std::string                                              mSenderQuery;              // Used to execute a query script on sender before sending message.
   std::string                                              mDestQuery;                // Used to execute a query script on destination before accepting the message.
   boost::shared_ptr<CGameObjectMessageProperties const>    mpProperties;
};

//------------------------------------------------------------------------------------------
// Used to supply info about sender of message
class ENGINE_API CGameObjectMessageInfo
{
public:
   CGameObjectMessageInfo(TComponentUniqueId const &sender, TComponentUniqueId const &originator)
   : mSender(sender)
   , mOriginator(originator)
   {
   };

   TComponentUniqueId      mSender;
   TComponentUniqueId      mOriginator;
};

//------------------------------------------------------------------------------------------

