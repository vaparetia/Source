 //----------------------------------------------------------------------------
// CShaderObjectFactory.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CShaderObjectFactory.h"

//----------------------------------------------------------------------------

#include "Engine/GameObjectSystem/CGameObjectRegisteredComponentGroupObjectFactories.h"
#include "Engine/Evaluators/CEngineEvaluatorFactory.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

// Used for registering factories for component groups/components
CGameObjectRegisteredComponentGroupObjectFactories gsComponentGroupObjectFactoriesShader;

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CGameObjectComponentProperties * const CShaderObjectFactory::BuildComponentProperties(CInputStream &inStream)
{
   uint32 const componentGroupHash = inStream.ReadUint32();
   uint16 const componentGroupSize = inStream.ReadUint16();

   CGameObjectApplyPropertiesData applyProperties(inStream, this->GetEvaluatorFactory());
   CGameObjectComponentProperties * const pProperties = gsComponentGroupObjectFactoriesShader.BuildGameObjectComponentProperties(componentGroupHash, applyProperties);
   if( pProperties )
      return pProperties;

   // Check to see if it was an unknown component group, if it's unknown we'll have to skip the data ourselves.
   if (!gsComponentGroupObjectFactoriesShader.ComponentGroupFactoryExists(componentGroupHash))
   {
      // Unknown component group, skip data
      inStream.Get(NULL, componentGroupSize);
   }

   return NULL;
}

//----------------------------------------------------------------------------

CGameObjectMessageProperties * const CShaderObjectFactory::BuildMessageProperties(CInputStream &inStream)
{
   return NULL;
}

//----------------------------------------------------------------------------

CGameObjectComponent * const CShaderObjectFactory::BuildComponent(boost::shared_ptr<CGameObjectComponentProperties const> pComponentProperties)
{
   return NULL;
}

//----------------------------------------------------------------------------

CGameObject * const CShaderObjectFactory::BuildGameObject(boost::shared_ptr<CGameObjectProperties const> pGameObjectProperties)
{
   return NULL;
}

//----------------------------------------------------------------------------

IEvaluatorFactory const & CShaderObjectFactory::GetEvaluatorFactory()
{
   return GetEngineEvaluatorFactory();
}

//----------------------------------------------------------------------------


