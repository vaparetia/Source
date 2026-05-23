//------------------------------------------------------------------------------------------
// NJKEShaderCommon.cpp
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "NJKEShaderCommon.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//------------------------------------------------------------------------------------------

NJKEShaderCommon::SEnvMapParameters::SEnvMapParameters()
: mEnvMapTexture( CShaderCRCs::GetParameterCRC( "g_EnvMap" ) )
, mGlossFactor( CShaderCRCs::GetParameterCRC( "g_EnvMapGlossFactor" ) )
, mGlossAdd( CShaderCRCs::GetParameterCRC( "g_EnvMapGlossAdd" ) )
{
}

//------------------------------------------------------------------------------------------

NJKEShaderCommon::SFogParameters::SFogParameters()
: mFogColor( CShaderCRCs::GetParameterCRC( "g_FogColor" ) )
, mFogParams( CShaderCRCs::GetParameterCRC( "g_FogParams" ) )
{
}

//------------------------------------------------------------------------------------------

void NJKEShaderCommon::AddFogParameters( CRenderBackend const &backend, SFogParameters const &fog, CShaderParameterBuffer &parameters )
{
   if ( backend.GetFogEnabled() )
   {
      parameters.AddParameterGamma( fog.mFogColor, backend.GetFogColor(), kSCS_Gamma );

#if BPE_TARGET == BPE_TARGET_WIN32
      parameters.AddParameter( fog.mFogParams, backend.GetWin32FogParams() );
#endif
   }
}
