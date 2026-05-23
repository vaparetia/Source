//------------------------------------------------------------------------------------------
// RVLCRenderBackend.h
// Bluepoint
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Backend/RVL/CRVLRenderState.h"

#include <revolution/gx/GXStruct.h>
#include <revolution/gx/GXEnum.h>

//----------------------------------------------------------------------------

class CShaderVertexDataBinding;

class RENDERER_API CRenderBackend : public CBaseRenderBackend
{
   typedef CBaseRenderBackend inherited;
   friend class CBaseRenderBackend;
public:
   enum EProjectionType
   {
      kPT_Orthogonal,
      kPT_Perspective
   };

   explicit CRenderBackend(IResourcePool & resourcePool,
                           SRenderInitialization const & initFlags);
   ~CRenderBackend();

   void SetFogEnabled( bool const enable );
   void SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color );
   bool GetFogEnabled() const { return false; }
   CColorf const GetFogColor() const { return CColorf::White(); }

   void RenderQuad2d(CVector2 const & start,
      CVector2 const & end,
      CColor const & color);

   virtual void RenderLine2d(CVector2 const & start,
      CVector2 const & end,
      CColor const & color);
   
   void RenderLine3d(CVector3 const & start,
      CVector3 const & end,
      CColor const & color);

   void RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
      CShaderVertexDataBinding binding, 
      void const * pData, 
      uint32 const numVertices);

   void  SetVertexData(CVertexData const & vertexData, uint64 const vertexDataHash);

   void RenderDisplayList( void const *pDisplayList, uint32 const displayListSize );

   void ResolveRenderTarget(CBaseTexture * pRenderTexture);
   void SetRenderTarget( SRenderTarget const &renderTarget );
   SRenderTarget const &GetCurrentRenderTarget() const;

   void Clear(int const clearFlags, CColor const color, real32 const z = 1.0f, int const stencil = 0);

   // Allocate a block of memory from the given pool that is fixed (i.e. unrelocateable)
   CRenderHWAllocator::SHandle const * AllocFixed(int const size, ERenderMemory const memoryType, ERenderMemory const fallbackMemory = kRM_Invalid);
   void Free(CRenderHWAllocator::SHandle const * pHandle);

   void BeginScene();
   void EndScene();
   void Present();

   void SetProjectionMatrix(CMatrix4 const &matrix, EProjectionType const pt );
   void SetModelMatrix( CMatrix34 const &matrix );
   void SetVertexDescriptor( CShaderVertexDataBinding const & );

   CMatrix4 const GetViewMatrix() const { return CMatrix4::FromMatrix34( mViewMatrix ); }
   CMatrix4 const GetCameraMatrix() const { return CMatrix4::FromMatrix34( mCameraMatrix ); }

   void SetPixelStateSlow( CRVLRenderState::SPixelState const &pixelState );
   void SetPixelState( CRVLRenderState::TPixelStateHandle const &pixelState );

   void SetVariableUnitStatesSlow( CRVLRenderState::SVariableUnitStates const &unitStates );
   void SetVariableUnitStates( CRVLRenderState::TVariableUnitStatesHandle const &unitStates );

   void SetTevStageSlow( GXTevStageID tevStage, CRVLRenderState::STevStageState const &state );
   void SetTevStage( GXTevStageID tevStage, CRVLRenderState::TTevStageStateHandle const &state );

   void SetTexCoordGenSlow( GXTexCoordID texCoord, CRVLRenderState::STexCoordGen const &state ); 
   void SetTexCoordGen( GXTexCoordID texCoord, CRVLRenderState::TTexcoordGenHandle const &state ); 
private:

   void InitFrameBuffers();
   void FlushRenderModeChangesToGX();
   void FlushPNMatrixState();

   static GXBlendFactor const GetGXBlendFactor( EBlendFunc const blendFunction );

   boost::scoped_ptr< uint8 > mpFIFO;
   boost::scoped_ptr< uint8 > mpXFB1;
   boost::scoped_ptr< uint8 > mpXFB2;

   int mCurrentXFB;

   GXRenderModeObj mRenderMode;

   CMatrix34 mViewMatrix;
   CMatrix34 mCameraMatrix;
   CMatrix34 mModelMatrix;

   CRVLRenderState::CStateHandle::TUniqueID mPixelStateHandle;
   CRVLRenderState::CStateHandle::TUniqueID mVariableUnitStatesHandle;
   CRVLRenderState::CStateHandle::TUniqueID mTevStageStateHandle[ GX_MAX_TEVSTAGE ];
   CRVLRenderState::CStateHandle::TUniqueID mTexCoordGenHandle[ GX_MAX_TEXCOORDGEN ];
};

//------------------------------------------------------------------------------------------
