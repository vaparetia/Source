//----------------------------------------------------------------------------
// RVLCCompiledShaderCache.cpp
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "RVLCCompiledShaderCache.h"

//----------------------------------------------------------------------------

int const CCompiledShader::Begin()
{
   return 1;
}

//----------------------------------------------------------------------------

void CCompiledShader::End()
{
}

//----------------------------------------------------------------------------

int const CCompiledShader::BeginPass( int const currentPass )
{
   return 0;
}

//----------------------------------------------------------------------------

void CCompiledShader::EndPass()
{
}

//----------------------------------------------------------------------------

void CCompiledShader::BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
   uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
   uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize)
{
}

//----------------------------------------------------------------------------

void CCompiledShader::EndShaderBatch()
{
}

//----------------------------------------------------------------------------

int const CCompiledShader::GetCurrentPassIndex() const
{
   return 0;
}

//----------------------------------------------------------------------------

int const CCompiledShader::GetPassCount() const
{
   return 1;
}

//----------------------------------------------------------------------------

CCompiledShaderCache::CCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

CCompiledShaderCache::~CCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::SetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
}

//----------------------------------------------------------------------------

CCompiledShader *CCompiledShaderCache::CurrentShader()
{
   return NULL;
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::FlushAllShaders()
{
}