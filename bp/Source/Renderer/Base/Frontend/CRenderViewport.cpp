//----------------------------------------------------------------------------
// CRenderViewport.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CRenderViewport.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CRenderViewport::CRenderViewport()
:  mRenderTarget()
,  mTransform( CMatrix34::Identity() )
,  mProjectionMatrix(CMatrix4::Identity())
,  mAspect( 1.33f )
,  mFOV( CAngle::FromDegrees(45.0f) )
,  mNearClip( 1.0f )
,  mFarClip( 10000.0f )
{
}

//----------------------------------------------------------------------------

CRenderViewport::~CRenderViewport()
{
}
