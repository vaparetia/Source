//------------------------------------------------------------------------------------------
// CRVLShaderOneTex.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CRVLShaderOneTex.h"
#include "Renderer/Base/ShaderObjects/RVLShader/CCPLdrRVLShaderOneTex.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include <revolution/gx.h>

namespace
{
   CRVLRenderState::TPixelStateHandle const skPixelStateHandle( CRVLRenderState::SPixelState(
      GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR,
      CRVLRenderState::SPixelState::kDepthCompare_Enabled, GX_LEQUAL, CRVLRenderState::SPixelState::kDepthUpdate_Enabled,
      CRVLRenderState::SPixelState::kColorUpdate_Enabled, CRVLRenderState::SPixelState::kAlphaUpdate_Enabled, CRVLRenderState::SPixelState::kDstAlpha_FromTev,
      GX_ALWAYS ) );
   CRVLRenderState::TTevStageStateHandle const skFirstPassTextureStateHandle( CRVLRenderState::STevStageState(
      GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD0, 
      GX_TEXMAP0, 
      GX_COLOR_NULL ) );
   CRVLRenderState::TTevStageStateHandle const skFirstPassNoTextureStateHandle( CRVLRenderState::STevStageState(
      GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD_NULL, 
      GX_TEXMAP_NULL, 
      GX_COLOR_NULL ) );
   CRVLRenderState::TTevStageStateHandle const skLightmapPassStateHandle( CRVLRenderState::STevStageState(
      GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD1, 
      GX_TEXMAP1, 
      GX_COLOR_NULL ) );
   CRVLRenderState::TTexcoordGenHandle const &skFirstPassTexcoordGen = CRVLRenderState::GetDefaultTexcoordGenForTexture( 0 );
   CRVLRenderState::TTexcoordGenHandle const &skSecondPassTexcoordGen = CRVLRenderState::GetDefaultTexcoordGenForTexture( 1 );
   CRVLRenderState::TVariableUnitStatesHandle const &skUnitStatesNoLightmap = CRVLRenderState::GetUnitStatesForSameTevsAndTexGensNoChan( 1 );
   CRVLRenderState::TVariableUnitStatesHandle const &skUnitStatesLightmap = CRVLRenderState::GetUnitStatesForSameTevsAndTexGensNoChan( 2 );
}

//----------------------------------------------------------------------------

CRVLShaderOneTex::CRVLShaderOneTex( TShaderProperties const &properties )
: inherited( properties )
{
}

//----------------------------------------------------------------------------

void CRVLShaderOneTex::Bind( CMaterialFlags const * const pMaterialFlags )
{
   CCPLdrRVLShaderOneTex const *pProperties = static_cast<CCPLdrRVLShaderOneTex const *>( mProperties.get() );

   gpRenderBackend->SetPixelState( skPixelStateHandle );

   if ( pProperties->mTexture_Resource.is_initialized() )
   {
      gpRenderBackend->SetTevStage( GX_TEVSTAGE0, skFirstPassTextureStateHandle );
      gpRenderBackend->SetTexCoordGen( GX_TEXCOORD0, skFirstPassTexcoordGen );
      pProperties->mTexture_Resource->GetPtr()->SetTexture( 0 );
   }
   else
   {
      gpRenderBackend->SetTevStage( GX_TEVSTAGE0, skFirstPassNoTextureStateHandle );
   }

   if ( pProperties->mCommonLightmapProperties.mLightmapTexture_Flat_Resource.is_initialized() )
   {
      pProperties->mCommonLightmapProperties.mLightmapTexture_Flat_Resource->GetPtr()->SetTexture( 1 );

      gpRenderBackend->SetTevStage( GX_TEVSTAGE1, skLightmapPassStateHandle );
      gpRenderBackend->SetTexCoordGen( GX_TEXCOORD1, skSecondPassTexcoordGen );
      gpRenderBackend->SetVariableUnitStates( skUnitStatesLightmap );
   }
   else
   {
      gpRenderBackend->SetVariableUnitStates( skUnitStatesNoLightmap );
   }
}