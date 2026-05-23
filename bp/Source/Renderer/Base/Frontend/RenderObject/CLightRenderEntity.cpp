//----------------------------------------------------------------------------
// CLightRenderEntity.cpp
// Bluepoint
// Copyright 2004
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CLightRenderEntity.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CLightRenderEntity) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CLightRenderEntity::CLightRenderEntity()
:  CRenderObject( kType_Light )
,  mLight()
{
}

//----------------------------------------------------------------------------

CLightRenderEntity::CLightRenderEntity( CLight const & light )
:  CRenderObject( kType_Light )
,  mLight( light )
{
}

//----------------------------------------------------------------------------

CLightRenderEntity::~CLightRenderEntity()
{
}

//----------------------------------------------------------------------------

