//------------------------------------------------------------------------------------------
// CGameObjectDebugLog.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/GameObjectSystem/TGameObjectTypes.h"
#include "boost/scoped_ptr.hpp"

//------------------------------------------------------------------------------------------

class COutputStream;
class TiXmlNode;
class CBaseGameManager;
class CGameObjectMessage;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectDebugLog
{
public:
   CGameObjectDebugLog(CBaseGameManager const * pManager);
   ~CGameObjectDebugLog();

   void        LogInfoMessage(char const * const pFormatString, ...) const;
   void        LogWarningMessage(char const * const pFormatString, ...) const;
   void        LogErrorMessage(char const * const pFormatString, ...) const;
   // Will choose color based on level unless explicitly specified
   // Color not currently supported
   void        LogMessage(std::string const &msg, uint32 const level = 0, uint32 color = 0) const;
   // Function takes ownership of node
   void        LogInfo(TiXmlNode *pNode) const;
   void        WriteLogInfoToStream(COutputStream &outStream);
   void        SetMessageLogging(bool enable);

   // Used for logging messages
   uint32      MessageHandlerCallback(TComponentId const &destId, CGameObjectMessage const &message, TComponentUniqueId const &senderId, TComponentUniqueId const &originatorId);

private:
   TiXmlNode * GetFrameNode() const;

   mutable boost::shared_ptr<TiXmlNode>         mpLogInfo;  // auto_ptr won't copy with optional due to const.
   CBaseGameManager const *                     mpManager;
};

//------------------------------------------------------------------------------------------
