//----------------------------------------------------------------------------
// CMeshRenderEntity.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Engine/Resource/CResourceManager.h"
#include "Engine/Math/CMatrix4.h"
#include "Engine/Math/CMatrix34.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableMasterRenderEntity.h"

#include "Renderer/Base/Primitive/CVertexBuffer.h"

//----------------------------------------------------------------------------

class CBaseTexture;
class CMesh;
class COverridenMeshBuffers;

//----------------------------------------------------------------------------

class CMeshRenderEntity : public CDrawableMasterRenderEntity
{
public:
   RENDERER_API explicit CMeshRenderEntity(CResource const &ownerResource, CMesh * pMesh);
   RENDERER_API virtual ~CMeshRenderEntity();

   RENDERER_API virtual void        RenderGeometry(CShaderVertexDataBinding const & vertexDataBinding);

   CMesh *                          Mesh()                                       { return mpMesh; }
   CMesh const *                    GetMesh() const                              { return mpMesh; }
   CResource const &                GetMeshOwnerToken() const                    { return mMeshOwnerToken; }
   virtual CMatrix34 const &        GetTransform() const                         { return mTransform; }
   virtual void                     SetTransform(CMatrix34 const & transform)    { mTransform = transform; }
   
public:
   CMatrix34                           mTransform;

#if 0
   CDynamicVertexBuffer                mDynamicData;
#endif
   CMatrix4                            mProjectionMatrix;
   CVector3                            mLightDir0, mLightDir1;
   CVector4                            mLightColor0, mLightColor1;
   CVector4                            mAmbient;
   real32                              mLODAlpha;

   CMesh *                             mpMesh;
   CResource                           mMeshOwnerToken;
};

