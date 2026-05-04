//------------------------------------------------------------------------------------------
// CShaderObjectFactory.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/GameObjectSystem/CGameObjectComponent.h"
#include "Engine/GameObjectSystem/CGameObjectComponentMethods.h"
#include "Engine/GameObjectSystem/CGameObjectComponentProperties.h"
#include "Engine/GameObjectSystem/CGameObjectMessageProperties.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"

//------------------------------------------------------------------------------------------

class IResourcePool;

//------------------------------------------------------------------------------------------

class CShaderObjectFactory : public IGameObjectFactory
{
public:
   virtual CGameObjectComponentProperties * const  BuildComponentProperties(CInputStream &inStream);
   virtual CGameObjectMessageProperties * const    BuildMessageProperties(CInputStream &inStream);

   virtual CGameObjectComponent * const            BuildComponent(boost::shared_ptr<CGameObjectComponentProperties const> pComponentProperties);
   virtual CGameObject * const                     BuildGameObject(boost::shared_ptr<CGameObjectProperties const> pGameObjectProperties);
   virtual IEvaluatorFactory const &               GetEvaluatorFactory();
};

//----------------------------------------------------------------------------
