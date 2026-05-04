//------------------------------------------------------------------------------------------
// CRenderViewport.h
// Bluepoint
// Copyright 2008
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/Math/CMatrix34.h"

//------------------------------------------------------------------------------------------

class CRenderViewport
{
public:
   RENDERER_API explicit CRenderViewport();
   RENDERER_API virtual ~CRenderViewport();

   void SetRenderTarget(SRenderTarget const & renderTarget) { mRenderTarget = renderTarget; }
   SRenderTarget const &  GetRenderTarget() const { return mRenderTarget; }

   void SetRenderTargetDepthRange( real32 const minZ, real32 const maxZ ) { mRenderTarget.mMinZ = minZ; mRenderTarget.mMaxZ = maxZ; }

   CMatrix34 const & GetTransform() const { return mTransform; }
   void              SetTransform( CMatrix34 const & transform ) { mTransform = transform; }

   void SetProjectionMatrix(CMatrix4 const & projection) { mHasCustomProjection = true; mProjectionMatrix = projection; }
   bool HasCustomProjection() const { return mHasCustomProjection; }
   CMatrix4 const & GetProjectionMatrix() const { return mProjectionMatrix; }

   real32 const GetAspect() const { return mAspect; }
   void SetAspect( real32 const value ) { mAspect = value; }

   CAngle const GetFOV() const { return mFOV; }
   void SetFOV( CAngle const value ) { mFOV = value; }

   real32 const GetNearClipPlane() const { return mNearClip; }
   void SetNearClipPlane( real32 const value ) { mNearClip = value; }

   real32 const GetFarClipPlane() const { return mFarClip; }
   void SetFarClipPlane( real32 const value ) { mFarClip = value; }

private:
   SRenderTarget  mRenderTarget;
   CMatrix34      mTransform;
   CMatrix4       mProjectionMatrix;
   real32         mAspect;
   CAngle         mFOV;
   real32         mNearClip, mFarClip;
   bool           mHasCustomProjection;
};


