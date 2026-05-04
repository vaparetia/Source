//------------------------------------------------------------------------------------------
// CEditorLight.cpp
// Bluepoint
// Abstract
// Copyright 2003
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CEditorLight.h"

//------------------------------------------------------------------------------------------

#include "Engine/Math/CloseEnough.h"
#include "Engine/Mechanics/TTokenSet.h"
#include "Engine/Mechanics/CXMLTree.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

static const TTokenSet<CEditorLight::EType> skLightTypeTokens[] =
{
   "Occlusion",   CEditorLight::kType_Occlusion,
   "Ambient",     CEditorLight::kType_Ambient,
   "Point",       CEditorLight::kType_Point,
   "Directional", CEditorLight::kType_Directional,
   "Spot",        CEditorLight::kType_Spot,
   NULL,          CEditorLight::kType_Invalid
};

static const TTokenSet<CEditorLight::EFalloff> skLightFalloffTokens[] =
{
   "None",        CEditorLight::kFalloff_None,
   "Linear",      CEditorLight::kFalloff_Linear,
   "Quadratic",   CEditorLight::kFalloff_Quadratic,
   "Cubic",       CEditorLight::kFalloff_Cubic,
   NULL,          CEditorLight::kFalloff_Invalid
};

//------------------------------------------------------------------------------------------

CEditorLight::CEditorLight( CMatrix34 const & transform,
                            EType const type,
                            CVector3 const & color,
                            real32 const intensity,
                            EFalloff const falloff,
                            CAngle const & coneAngle,
                            CAngle const & penumbraAngle,
                            real32 const dropOff,
                            bool const castShadows )
:  CLight( transform,
           type,
           color,
           intensity,
           falloff,
           coneAngle,
           penumbraAngle,
           dropOff,
           castShadows )
{
}

//------------------------------------------------------------------------------------------

CEditorLight::CEditorLight( CXMLTree const & tree )
:  CLight( CMatrix34::Identity(), 
           kType_Invalid,
           CVector3::Zero(),
           0.0f,
           kFalloff_None,
           CAngle::FromDegrees( 0.0f ),
           CAngle::FromDegrees( 0.0f ),
           0.0f,
           false )
{
   ParseTree( tree );
}

//------------------------------------------------------------------------------------------

void CEditorLight::ParseTree( CXMLTree const & tree )
{
   mType = skLightTypeTokens->GetToken( tree.GetKeyValueSafe( "Type", "Point" ) );
   
   CVector3 position = CVector3::Zero();
   CVector3 orientation = CVector3::ZAxis();

   tree.GetKeyValue( "Position", position );
   tree.GetKeyValue( "Orientation", orientation );
   
   mTransform = CMatrix34::LookAt( position, position + orientation );

   tree.GetKeyValue( "Color", mColor );
   tree.GetKeyValue( "Intensity", mIntensity );
   
   mFalloff = skLightFalloffTokens->GetToken( tree.GetKeyValueSafe( "Falloff", "None" ) );
   tree.GetKeyValue( "CastsShadows", mCastShadows );

   tree.GetKeyValue( "ConeAngle", mConeAngle );
   tree.GetKeyValue( "PenumbraAngle", mPenumbraAngle );
   tree.GetKeyValue( "Dropoff", mDropOff );

   tree.GetKeyValue( "IsStatic", mIsStatic );

}

//------------------------------------------------------------------------------------------
   
void CEditorLight::PutTo( CXMLTree & tree ) const
{
   tree.SetKeyValue( "Type", skLightTypeTokens->GetNameByToken( mType ) );
   tree.SetKeyValue( "Position", mTransform.GetTranslation() );
   tree.SetKeyValue( "Orientation", mTransform.GetForward() );
   tree.SetKeyValue( "Color", mColor );
   tree.SetKeyValue( "Intensity", mIntensity );
   tree.SetKeyValue( "Falloff", skLightFalloffTokens->GetNameByToken( mFalloff ) );
   
   tree.SetKeyValue( "CastsShadows", mCastShadows );
   tree.SetKeyValue( "IsStatic", mIsStatic );
   
   if( mType == kType_Spot )
   {
      tree.SetKeyValue( "ConeAngle", mConeAngle );
      tree.SetKeyValue( "PenumbraAngle", mPenumbraAngle );
      tree.SetKeyValue( "Dropoff", mDropOff );
   }

}

