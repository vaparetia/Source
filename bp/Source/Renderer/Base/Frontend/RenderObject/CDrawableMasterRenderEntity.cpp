//----------------------------------------------------------------------------
// CDrawableMasterRenderEntity.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CDrawableMasterRenderEntity.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/GameObjectSystem/CGameObjectComponentProperties.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CDrawableMasterRenderEntity) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CDrawableMasterRenderEntity::CDrawableMasterRenderEntity(EType const type )
:  CDrawableRenderEntity(type)
,  mPreRenderTimeStamp(-1)
{
   // Master render entities own themselves
   mpOwner = this;
}

//----------------------------------------------------------------------------

bool CDrawableMasterRenderEntity::PreRenderGeometry()
{
   // No point adding this to the task queue, it's too small.
   mPreRenderTimeStamp = RenderBackend()->GetFrameCount();
   return true;
}
