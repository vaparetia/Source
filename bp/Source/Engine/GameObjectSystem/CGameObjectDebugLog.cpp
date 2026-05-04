//----------------------------------------------------------------------------
// CGameObjectDebugLog.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <sstream>
#include "boost/bind.hpp"
#include "Engine/GameObjectSystem/CGameObjectDebugLog.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"
#include "Engine/GameObjectSystem/CGameObjectMessageHandler.h"
#include "Engine/GameObjectSystem/CGameObjectMessageProperties.h"
#include "Engine/Graphics/CColor.h"

#include "Engine/Mechanics/TinyXml/TinyXml.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

//----------------------------------------------------------------------------

CGameObjectDebugLog::CGameObjectDebugLog(CBaseGameManager const * pManager)
: mpManager(pManager)
{
}

//----------------------------------------------------------------------------

CGameObjectDebugLog::~CGameObjectDebugLog()
{
   SetMessageLogging(false);
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::LogInfoMessage(char const * const pFormatString, ...) const
{
   va_list args;
   va_start( args, pFormatString );

   LogMessage(CStringExtras::Stringize_s(pFormatString, args), 0); 
   va_end( args );
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::LogWarningMessage(char const * const pFormatString, ...) const
{
   va_list args;
   va_start( args, pFormatString );

   LogMessage(CStringExtras::Stringize_s(pFormatString, args), 0); 
   va_end( args );
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::LogErrorMessage(char const * const pFormatString, ...) const
{
   va_list args;
   va_start( args, pFormatString );

   LogMessage(CStringExtras::Stringize_s(pFormatString, args), 0); 
   va_end( args );
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::LogMessage(std::string const &msg, uint32 const level, uint32 color) const
{
   if (color == 0)
   {
      switch (level)
      {
      case 0:
         color = CColor::Black().GetARGB();
         break;
      case 1:
         color = CColor::Blue().GetARGB();
         break;
      case 2:
         color = CColor::Red().GetARGB();
         break;
      default:
         color = CColor::Green().GetARGB();
         break;
      }
   }
   
   TiXmlNode *pNode = GetFrameNode();
   TiXmlElement *pMsg = new TiXmlElement("ConsoleMessage");
   pMsg->SetAttribute("level", level);
   pMsg->SetAttribute("color", color);
   pMsg->SetAttribute("text", msg);
   pNode->LinkEndChild(pMsg);
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::LogInfo(TiXmlNode *pNode) const
{
   GetFrameNode()->LinkEndChild(pNode);
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::WriteLogInfoToStream(COutputStream &outStream)
{
   if (!mpLogInfo) return;

   std::ostringstream out;
   out << (*mpLogInfo.get());

   std::string data = out.str();

   // Have to use mem stream as string size header is dependent on size of string
   CGrowableMemoryOutStream memStream;
   memStream.Put(data);

   outStream.WriteUint32('GAME');
   outStream.WriteUint32(memStream.GetDataSize());
   outStream.Put(memStream.GetData(), memStream.GetDataSize());
   outStream.Flush();
}

//----------------------------------------------------------------------------

void CGameObjectDebugLog::SetMessageLogging(bool enable)
{
   if (mpManager)
   {
      // We've got a const cast, but we're not modifing the state of messages or manager.
      // I just don't want people constructing this class with a non-const manager and thinking 
      // that it's going to change state.
      CGameObjectMessageHandler &handler = const_cast<CBaseGameManager*>(mpManager)->MessageHandler();
      CGameObjectMessageHandler::TMessageCallback callback(fastdelegate::MakeDelegate(this, &CGameObjectDebugLog::MessageHandlerCallback));

      if (enable)
      {           
         handler.AddMessageCallback(callback);
      }
      else
      {
         handler.RemoveMessageCallback(callback);
      }
   }
}

//----------------------------------------------------------------------------

uint32 CGameObjectDebugLog::MessageHandlerCallback(TComponentId const &destId,
                                                   CGameObjectMessage const &message,
                                                   TComponentUniqueId const &senderId,
                                                   TComponentUniqueId const &originatorId)
{
   if (!mpManager) return 0;

   TiXmlElement * pMessage = new TiXmlElement("Message");

   CGameObjectComponent const * pSenderComponent = mpManager->GetComponentFromUniqueId(senderId);
   if (pSenderComponent)
   {
      pMessage->SetAttribute("eventName", pSenderComponent->GetEventTypeName(message.mEventId));

      CGameObject const * pSenderGameObject = pSenderComponent->GetGameObject();

      pMessage->SetAttribute("name", pSenderGameObject->GetProperties()->mEditorName);
      pMessage->SetAttribute("componentName", std::string(pSenderComponent->GetProperties()->GetComponentGroupName()) + ":" + std::string(pSenderComponent->GetProperties()->GetComponentTypeName()));
   }
   else
   {
      pMessage->SetAttribute("name", "NoSenderComponent");
   }

   // Recipients
   TConstComponentListTransient components;
   mpManager->GetComponentsFromUniqueOrEditorId(destId, components);
   for (TConstComponentListTransient::const_iterator it = components.begin(); it != components.end(); ++it)
   {
      CGameObjectComponent const * pComponent = *it;

      // We'll just use the message type from the first component as they will all be the same
      if (it == components.begin())
      {
         pMessage->SetAttribute("messageName", pComponent->GetMessageTypeName(message.mpProperties->GetMessageType()));
      }

      CGameObject const * pGameObject = pComponent->GetGameObject();

      TiXmlElement * pNode = new TiXmlElement("Receiver");

      pNode->SetAttribute("name", pGameObject->GetProperties()->mEditorName);
      pNode->SetAttribute("componentName", std::string(pComponent->GetProperties()->GetComponentGroupName()) + ":" + std::string(pComponent->GetProperties()->GetComponentTypeName()));
      pMessage->LinkEndChild(pNode);
   }
   LogInfo(pMessage);
   return 0;
}

//----------------------------------------------------------------------------

TiXmlNode * CGameObjectDebugLog::GetFrameNode() const
{
   if (!mpLogInfo)
   {
      uint32 const frameNum = mpManager ? mpManager->GetUpdateCount() : -1;
      TiXmlElement * pElement = new TiXmlElement("Frame");
      pElement->SetAttribute("frameNum", frameNum);
      mpLogInfo.reset(pElement);
   }
   return mpLogInfo.get();
}

//----------------------------------------------------------------------------
