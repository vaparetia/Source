//----------------------------------------------------------------------------
// CMeshRenderEntity.cpp
// Bluepoint
// Copyright 2004
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CMeshRenderEntity.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CMeshRenderEntity) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CMeshRenderEntity::CMeshRenderEntity(CResource const &meshOwnerToken, CMesh * pMesh)
:  CDrawableMasterRenderEntity(kType_RenderEntity)
,  mMeshOwnerToken(meshOwnerToken)
,  mpMesh(pMesh)
,  mTransform(CMatrix34::Identity())
,  mProjectionMatrix(CMatrix4::Identity())
,  mLightDir0(CVector3::Zero())
,  mLightDir1(CVector3::Zero())
,  mLightColor0(CVector4::Zero())
,  mLightColor1(CVector4::Zero())
,  mAmbient(CVector4::Zero())
,  mLODAlpha(1.0f)
{
}

//----------------------------------------------------------------------------

CMeshRenderEntity::~CMeshRenderEntity()
{
}

//----------------------------------------------------------------------------

void CMeshRenderEntity::RenderGeometry(CShaderVertexDataBinding const & /*vertexDataBinding*/)
{
}

//----------------------------------------------------------------------------

