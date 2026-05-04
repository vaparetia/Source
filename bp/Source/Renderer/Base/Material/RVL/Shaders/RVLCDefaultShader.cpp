//------------------------------------------------------------------------------------------
// CDefaultShader.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "RVLCDefaultShader.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/GameObjectSystem/CGameObjectComponentProperties.h"

#include <revolution/gx.h>

namespace
{
   CRVLRenderState::SPixelState const skPixelState(
      GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR,
      CRVLRenderState::SPixelState::kDepthCompare_Disabled, GX_LEQUAL, CRVLRenderState::SPixelState::kDepthUpdate_Disabled,
      CRVLRenderState::SPixelState::kColorUpdate_Enabled, CRVLRenderState::SPixelState::kAlphaUpdate_Enabled, CRVLRenderState::SPixelState::kDstAlpha_FromTev,
      GX_ALWAYS );

   CRVLRenderState::STevStageState const skFirstPassTextureState(
      GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD0, 
      GX_TEXMAP0, 
      GX_COLOR_NULL );

   CRVLRenderState::STevStageState const skFirstPassNoTextureState(
      GX_CC_ZERO, GX_CC_ONE, GX_CC_KONST, GX_CC_ONE, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD_NULL, 
      GX_TEXMAP_NULL, 
      GX_COLOR_NULL );

   CRVLRenderState::STevStageState const skFirstPassTextureStateIterated(
      GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD0, 
      GX_TEXMAP0, 
      GX_COLOR0A0 );

   CRVLRenderState::STevStageState const skFirstPassNoTextureStateIterated(
      GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ONE, GX_TEV_KCSEL_1,
      GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(),
      CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD_NULL, 
      GX_TEXMAP_NULL, 
      GX_COLOR0A0 );

   CRVLRenderState::TVariableUnitStatesHandle const &skUnitStatesNoChanHandle = CRVLRenderState::GetUnitStatesForSameTevsAndTexGensNoChan( 1 );
   CRVLRenderState::TVariableUnitStatesHandle const skUnitStatesWithChanHandle( CRVLRenderState::SVariableUnitStates( 1, 0, 1, 1 ) );
}

CDefaultShader::CDefaultShader()
: inherited( TShaderProperties() )
, mHasColorStream( false )
, mUseKonstColor( false )
, mBlendMode( kBM_Replace )
, mZEnable( false )
{
}

void CDefaultShader::Bind(const CMaterialFlags *const pMaterialFlags)
{
   CRVLRenderState::SPixelState pixelState = skPixelState;

   if ( !mZEnable )
   {
      pixelState.mDepthCompare = CRVLRenderState::SPixelState::kDepthCompare_Disabled;
   }

   switch ( mBlendMode )
   {
   case kBM_Replace:
      // Defaults to replace
      break;
   case kBM_Additive:
      pixelState.mType = GX_BM_BLEND;
      pixelState.mSrcFactor = GX_BL_ONE;
      pixelState.mDstFactor = GX_BL_ONE;
      break;
   case kBM_Alpha:
      pixelState.mType = GX_BM_BLEND;
      pixelState.mSrcFactor = GX_BL_SRCALPHA;
      pixelState.mDstFactor = GX_BL_INVSRCALPHA;
      break;
   case kBM_PremultipliedAlpha:
      pixelState.mType = GX_BM_BLEND;
      pixelState.mSrcFactor = GX_BL_ONE;
      pixelState.mDstFactor = GX_BL_INVSRCALPHA;
      break;
   }

   gpRenderBackend->SetPixelStateSlow( pixelState );

   if ( mTexture.is_initialized() )
   {
      CBaseTexture const *pTexture = mTexture.get().GetPtr();
      gpRenderBackend->SetTexCoordGen( GX_TEXCOORD0, CRVLRenderState::GetDefaultTexcoordGenForTexture( 0 ) );
      pTexture->SetTexture( 0 );
   }

   CRVLRenderState::STevStageState const *pSrcTevState;

   if ( mTexture.is_initialized() )
   {
      pSrcTevState = ( mHasColorStream ) ? &skFirstPassTextureStateIterated : &skFirstPassTextureState;
   }
   else
   {
      pSrcTevState = ( mHasColorStream ) ? &skFirstPassNoTextureStateIterated : &skFirstPassNoTextureState;
   }
   
   CRVLRenderState::STevStageState tevState = *pSrcTevState;

   if ( mHasColorStream )
   {
      GXSetChanCtrl( GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE );
      gpRenderBackend->SetVariableUnitStates( skUnitStatesWithChanHandle );
   }
   else
   {
      if ( mConstColor.is_initialized() )
      {
         tevState.mKColorSel = GX_TEV_KCSEL_K0;
         tevState.mKAlphaSel = GX_TEV_KASEL_K0_A;

         GXSetTevKColor( GX_KCOLOR0, mConstColor->AsGXColor() );
      }
      else if ( mUseKonstColor )
      {
         tevState.mKColorSel = GX_TEV_KCSEL_K0;
         tevState.mKAlphaSel = GX_TEV_KASEL_K0_A;
      }

      gpRenderBackend->SetVariableUnitStates( skUnitStatesNoChanHandle );
   }

   gpRenderBackend->SetTevStageSlow( GX_TEVSTAGE0, tevState );
}