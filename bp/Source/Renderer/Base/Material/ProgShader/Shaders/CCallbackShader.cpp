//------------------------------------------------------------------------------------------
// CCallbackShader.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CCallbackShader.h"

//------------------------------------------------------------------------------------------

#include "Engine/Resource/CResourceManager.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CCallbackShader::CCallbackShader()
: inherited(TShaderProperties()) 
, mSort(false)
, mpUserData(NULL)
{
}

//------------------------------------------------------------------------------------------

CCallbackShader::~CCallbackShader()
{
}

//------------------------------------------------------------------------------------------

void CCallbackShader::Bind(SRenderEntry const * pFirstObject)
{
   if (!mBindDelegate.empty()) 
   {
      mBindDelegate(mpUserData, *this, pFirstObject);
   }
   return;
}

//----------------------------------------------------------------------------

void CCallbackShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( !mBuildMaterialConstantParameterDelegate.empty() )
   {
      mBuildMaterialConstantParameterDelegate(mpUserData, colorSpace, parameters);
   }

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CCallbackShader::BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( !mBuildRuntimeParameterDelegate.empty() )
   {
      mBuildRuntimeParameterDelegate(mpUserData, object, parameterType, colorSpace, parameters);
   }

   inherited::BuildRuntimeParameters(pass, object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

uint64 CCallbackShader::GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const
{
   return (uint64)uniqueId;
}

