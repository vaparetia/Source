//------------------------------------------------------------------------------------------
// CGameObjectMessage.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/GameObjectSystem/CGameObjectMessage.h"
#include "Engine/GameObjectSystem/CGameObjectMessageProperties.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"
#include "Engine/Mechanics/CCRC.h"

//------------------------------------------------------------------------------------------

CGameObjectMessage::CGameObjectMessage(TScriptEventId const eventId, 
                                       TComponentId const &destId, 
                                       boost::shared_ptr<CGameObjectMessageProperties const> const &pProperties,
                                       real32 const triggerTime, 
                                       int32 const priority,
                                       std::string const &senderQuery,
                                       std::string const &destQuery)
:  mEventId(eventId)
,  mDestId(destId)
,  mDestEditorId(destId)
,  mTriggerTime(triggerTime)
,  mPriority(priority)
,  mSenderQuery(senderQuery)
,  mDestQuery(destQuery)
,  mpProperties(pProperties)
{
}

//------------------------------------------------------------------------------------------

CGameObjectMessage::~CGameObjectMessage()
{
}

//------------------------------------------------------------------------------------------

CGameObjectMessage const CGameObjectMessage::FMessageFactory(CInputStream &inStream, IGameObjectFactory &factory)
{
   TScriptEventId const eventId = inStream.ReadUint32();
   TComponentId const destId = TComponentId(inStream);
   real32 const triggerTime = inStream.ReadReal32();
   int32 const priority = inStream.ReadInt32();
   std::string const senderQuery = inStream.ReadString();
   std::string const destQuery = inStream.ReadString();

   CGameObjectMessageProperties const * const pMessageProperties = factory.BuildMessageProperties(inStream);

   return CGameObjectMessage( eventId, 
                              destId, 
                              boost::shared_ptr<CGameObjectMessageProperties const>(pMessageProperties),
                              triggerTime, 
                              priority, 
                              senderQuery, 
                              destQuery);
}

//------------------------------------------------------------------------------------------

