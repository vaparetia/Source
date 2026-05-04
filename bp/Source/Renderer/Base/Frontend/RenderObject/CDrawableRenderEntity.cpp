//----------------------------------------------------------------------------
// CDrawableRenderEntity.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CDrawableRenderEntity.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CDrawableRenderEntity) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CDrawableRenderEntity::CDrawableRenderEntity(EType const type)
:  CRenderObject(type)
,  mpShaderInstance(NULL)
,  mpOwner(NULL)
{
}

//----------------------------------------------------------------------------

CDrawableRenderEntity::~CDrawableRenderEntity()
{
}
