//------------------------------------------------------------------------------------------
// IGameObjectFactory.h
// Interface class for building component/message properties, game objects and components.
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "boost/shared_ptr.hpp"
//------------------------------------------------------------------------------------------

class CGameObjectProperties;
class CGameObjectComponentProperties;
class CGameObjectMessageProperties;

class CGameObject;
class CGameObjectComponent;
class CGameObjectMessage;
class IEvaluatorFactory;

//------------------------------------------------------------------------------------------

class ENGINE_API IGameObjectFactory
{
public:
   virtual ~IGameObjectFactory() {};

   virtual CGameObjectComponentProperties * const  BuildComponentProperties(CInputStream &inStream) = 0;
   virtual CGameObjectMessageProperties * const    BuildMessageProperties(CInputStream &inStream) = 0;

   virtual CGameObjectComponent * const            BuildComponent(boost::shared_ptr<CGameObjectComponentProperties const> pComponentProperties) = 0;
   virtual CGameObject * const                     BuildGameObject(boost::shared_ptr<CGameObjectProperties const> pGameObjectProperties) = 0;
   virtual IEvaluatorFactory const &               GetEvaluatorFactory() = 0;
private:
};

//------------------------------------------------------------------------------------------
