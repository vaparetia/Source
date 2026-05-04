//----------------------------------------------------------------------------
// RVLCRenderBackend.cpp
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Primitive/RVL/RVLCVertexBuffer.h"
#include "Renderer/Base/Backend/RVL/RVLCTexture.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"
#include "Renderer/Base/Primitive/CVertexData.h"

#include "Engine/Input/CInputGenerator.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CTaskQueue.h"

#include <revolution/gx.h>
#include <revolution/vi.h>
#include <revolution/sc.h>
#include <revolution/mtx.h>

#define RENDERBACKEND_SUPPORTS_ALPHA_TEST() 1

using namespace std;

//----------------------------------------------------------------------------


CRenderBackend::CRenderBackend(IResourcePool & resourcePool,
                               SRenderInitialization const &initFlags)
: CBaseRenderBackend(resourcePool, initFlags)
, mCurrentXFB( 0 )
, mViewMatrix( CMatrix34::Identity() )
, mCameraMatrix( CMatrix34::Identity() )
, mModelMatrix( CMatrix34::Identity() )
{
   InitFrameBuffers();

   InitDefaultObjects();
}

//----------------------------------------------------------------------------

CRenderBackend::~CRenderBackend()
{
}

//----------------------------------------------------------------------------

void CRenderBackend::InitFrameBuffers()
{
   static const size_t skFifoSize = 100 * 1024;

   mpFIFO.reset( reinterpret_cast<uint8 *>( bpe_rvl_alloc_aligned( 32, skFifoSize ) ) );

   GXInit( mpFIFO.get(), skFifoSize );

   if ( SCGetProgressiveMode() == SC_PROGRESSIVE_MODE_ON  )
   {
      memcpy( &mRenderMode, &GXNtsc480IntDf, sizeof( GXRenderModeObj ) );
   }
   else
   {
      memcpy( &mRenderMode, &GXNtsc480Prog, sizeof( GXRenderModeObj ) );
   }

   GXAdjustForOverscan( &mRenderMode, &mRenderMode, 0, 16 );
   GXSetCopyClear( (GXColor) { 192, 0, 192, 0 }, GX_MAX_Z24  );

   VIConfigure( &mRenderMode );
   VISetBlack( TRUE );
   VIFlush();

   FlushRenderModeChangesToGX();

   GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
   GXSetTexCoordGen( GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY );
   GXSetNumTexGens( 2 );
   
   GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC );
   GXSetTevColorOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
   GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST );
   GXSetTevAlphaOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
   GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
   GXSetTevKColor( GX_KCOLOR0, (GXColor) { 0, 255, 0, 255 } );
   GXSetTevKAlphaSel( GX_TEVSTAGE0, GX_TEV_KASEL_1 );
   GXSetTevKColorSel( GX_TEVSTAGE0, GX_TEV_KCSEL_K0 );

   GXSetNumTevStages( 1 );

   GXClearVtxDesc();
   GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
   GXSetVtxDesc( GX_VA_TEX0, GX_DIRECT );
   GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
   GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 );

   GXSetCullMode( GX_CULL_BACK );

   GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );
}

//----------------------------------------------------------------------------

void CRenderBackend::FlushRenderModeChangesToGX()
{
   GXSetScissor(  0, 0, mRenderMode.fbWidth, mRenderMode.efbHeight );

   GXSetDispCopySrc(0, 0, mRenderMode.fbWidth, mRenderMode.efbHeight);

   // call yscale after setting display copy source
   int n_xfb_lines =  // account for yscale in xfb height
      GXSetDispCopyYScale((f32)(mRenderMode.xfbHeight) /
      (f32)(mRenderMode.efbHeight));

   GXSetDispCopyDst(mRenderMode.fbWidth, n_xfb_lines);
   GXSetCopyFilter(mRenderMode.aa, mRenderMode.sample_pattern,
      GX_TRUE, mRenderMode.vfilter);

   if (mRenderMode.aa)
   {
      GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
   }
   else
   {
      GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
   }

   GXSetFieldMode(mRenderMode.field_rendering, mRenderMode.xfbHeight < mRenderMode.viHeight);

   int const xfbSize = VIPadFrameBufferWidth(mRenderMode.fbWidth) * n_xfb_lines * VI_DISPLAY_PIX_SZ;

   mpXFB1.reset( reinterpret_cast<uint8 *>( bpe_rvl_alloc_aligned_mem2( 32, xfbSize ) ) );
   mpXFB2.reset( reinterpret_cast<uint8 *>( bpe_rvl_alloc_aligned_mem2( 32, xfbSize ) ) );

   mBackbufferWidth = mRenderTargetWidth = mRenderMode.fbWidth;
   mBackbufferHeight = mRenderTargetHeight = mRenderMode.xfbHeight;

   mCurrentXFB = 0;
}

//----------------------------------------------------------------------------

void CRenderBackend::BeginScene()
{
}

//----------------------------------------------------------------------------

void CRenderBackend::EndScene()
{

}

//----------------------------------------------------------------------------

void CRenderBackend::Clear(int const clearFlags, CColor const color, real32 const z, int const stencil)
{
}

//----------------------------------------------------------------------------

void CRenderBackend::Present()
{
   InternalPresent();

   mFrustumNear = 0;

   mCurrentXFB ^= 1;

   void *currentXfb = ( mCurrentXFB == 0 ) ? mpXFB1.get() : mpXFB2.get();

   GXSetZMode( GX_TRUE, GX_LEQUAL, GX_TRUE );
   GXSetColorUpdate( GX_TRUE );
   GXSetAlphaUpdate( GX_TRUE );
   mPixelStateHandle = CRVLRenderState::CStateHandle::EmptyID();
   GXCopyDisp( currentXfb, true );
   GXDrawDone();

   VISetNextFrameBuffer( currentXfb );
   VISetBlack( FALSE );
   VIFlush();
   VIWaitForRetrace();
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveRenderTarget(CBaseTexture * pRenderTexture)
{

}

//----------------------------------------------------------------------------
   
void CRenderBackend::SetRenderTarget(SRenderTarget const & renderTarget)
{
   GXSetViewport( 0, 0, mRenderTargetWidth, mRenderTargetHeight, renderTarget.mMinZ, renderTarget.mMaxZ );
   GXSetScissor( 0, 0, mRenderTargetWidth, mRenderTargetHeight );
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderQuad2d( CVector2 const & start,
                                 CVector2 const & end,
                                 CColor const & color )
{
   GXSetTevKColor( GX_KCOLOR0, color.AsGXColor() );

   CShaderVertexDataBinding binding( kVDU_Position, kVDU_TexCoord0 );
   SetVertexDescriptor( binding );

   GXBegin( GX_TRIANGLEFAN, GX_VTXFMT0, 4 );
   {
      GXPosition3f32( start.GetX(), start.GetY(), mFrustumNear );
      GXTexCoord2f32( 0, 0 );
      GXPosition3f32( start.GetX(), end.GetY(), mFrustumNear );
      GXTexCoord2f32( 0, 1 );
      GXPosition3f32( end.GetX(), end.GetY(), mFrustumNear );
      GXTexCoord2f32( 1, 1 );
      GXPosition3f32( end.GetX(), start.GetY(), mFrustumNear );
      GXTexCoord2f32( 1, 0 );
   }
   GXEnd();
   GXDrawDone();
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderLine2d( CVector2 const & start,
                                 CVector2 const & end,
                                 CColor const & color )
{
   GXSetTevKColor( GX_KCOLOR0, color.AsGXColor() );

   CShaderVertexDataBinding binding( kVDU_Position );
   SetVertexDescriptor( binding );

   GXBegin( GX_LINES, GX_VTXFMT0, 2 );
   {
      GXPosition3f32( start.GetX(), start.GetY(), mFrustumNear );
      GXPosition3f32( end.GetX(), end.GetY(), mFrustumNear );
   }
   GXEnd();
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderLine3d( CVector3 const & start,
                                   CVector3 const & end,
                                   CColor const & color )
{
   GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST );
   GXSetTevKColorSel( GX_TEVSTAGE0, GX_TEV_KCSEL_K0 );
   GXSetTevKColor( GX_KCOLOR0, reinterpret_cast<GXColor const &>( color ) );

   CShaderVertexDataBinding binding( kVDU_Position );
   SetVertexDescriptor( binding );

   GXBegin( GX_LINES, GX_VTXFMT0, 2 );
   {
      GXPosition3f32( start.GetX(), start.GetY(), start.GetZ() );
      GXPosition3f32( end.GetX(), end.GetY(), end.GetZ() );
   }
   GXEnd();
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
                                                    CShaderVertexDataBinding binding, 
                                                    void const * pData, 
                                                    uint32 const numVertices)
{
   if ( binding == CShaderVertexDataBinding( kVDU_Position, kVDU_Color0, kVDU_TexCoord0 ) && type == CMeshChunk::kPrimitive_TriangleList )
   {
      real32 const *pReal32Data = reinterpret_cast<real32 const *>( pData );

      SetVertexDescriptor( binding );

      GXBegin( GX_TRIANGLES, GX_VTXFMT0, numVertices );
      for ( int i = 0; i < numVertices; ++i )
      {
         real32 const *pDataBase = pReal32Data + i * 6; // 6 = xyz + color + uv
         GXPosition3f32( pDataBase[0], pDataBase[1], pDataBase[2] );
         GXColor1u32( pDataBase[3] );
         GXTexCoord2f32( pDataBase[4], pDataBase[5] );
      }
      GXEnd();
      GXDrawDone();
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetVertexData(CVertexData const & vertexData, uint64 const vertexDataHash)
{
   for ( int i = 0; i < kVDU_CountBeforeNBT; ++i )
   {
      CVertexData::SVertexAttribute const &attr = 
         vertexData.GetAttribute( EVertexDataUsage(i) );

      if ( attr.mIsValid )
      {
         GXSetArray( GXAttr(i + GX_VA_POS), attr.mpBuffer->mpMemory->mpAddress + attr.mOffset, attr.mStride );
      }
   }

   CVertexData::SVertexAttribute const &nbtAttribute =  vertexData.GetAttribute( kVDU_NBT );
   if ( nbtAttribute.mIsValid )
   {
      GXSetArray( GX_VA_NBT, nbtAttribute.mpBuffer->mpMemory->mpAddress + nbtAttribute.mOffset, nbtAttribute.mStride );
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderDisplayList( void const *pDisplayList, uint32 const displayListSize )
{
   GXCallDisplayList( pDisplayList, displayListSize );

#if 0
   if ( OSTicksToSeconds( OSGetTick() ) & 1 )
   {
      // Draw cube every other second
      GXDrawCube();
   }
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color )
{
}

//----------------------------------------------------------------------------

void CRenderBackend::SetFogEnabled( bool const enable )
{
}

//----------------------------------------------------------------------------

SRenderTarget const &CRenderBackend::GetCurrentRenderTarget() const
{
   static SRenderTarget renderTarget;

   return renderTarget;
}

//----------------------------------------------------------------------------
CRenderHWAllocator::SHandle const * CRenderBackend::AllocFixed(int const size, ERenderMemory const memoryType, ERenderMemory const fallbackMemory )
{
   int i = 0;
   size_t const realSize = size + 32;

   void *alloced = NULL;
   for ( ERenderMemory allocType = memoryType; ( i < 2 ) && ( alloced == NULL ); allocType = fallbackMemory, ++i )
   {
      switch ( allocType )
      {
      case kRM_System:
         alloced = bpe_rvl_alloc_aligned( 32, realSize );
         break;
      case kRM_Video:
         alloced = bpe_rvl_alloc_aligned_mem2( 32, realSize );
         break;
      }
   }

   BPE_VERIFY( alloced != NULL, false, "Could not allocate render memory" );
   CRenderHWAllocator::SHandle *pHandle = new( alloced ) CRenderHWAllocator::SHandle();

   pHandle->mpAddress = reinterpret_cast<uint8 *>( alloced ) + 32;
   pHandle->mUserFlags = 0;

   return pHandle;
}

//----------------------------------------------------------------------------
void CRenderBackend::Free(CRenderHWAllocator::SHandle const * pHandle)
{
   free( const_cast<CRenderHWAllocator::SHandle *>( pHandle ) );
}

//----------------------------------------------------------------------------

GXBlendFactor const CRenderBackend::GetGXBlendFactor( EBlendFunc const blendFunc )
{
   switch ( blendFunc )
   {
   case kBF_Zero:
      return GX_BL_ZERO;
   case kBF_One:
      return GX_BL_ONE;
   case kBF_SrcColor:
      return GX_BL_SRCCLR;
   case kBF_InvSrcColor:
      return GX_BL_INVSRCCLR;
   case kBF_DstColor:
      return GX_BL_DSTCLR;
   case kBF_InvDstColor:
      return GX_BL_INVDSTCLR;
   case kBF_SrcAlpha:
      return GX_BL_SRCALPHA;
   case kBF_InvSrcAlpha:
      return GX_BL_INVSRCALPHA;
   case kBF_DstAlpha:
      return GX_BL_DSTALPHA;
   case kBF_InvDstAlpha:
      return GX_BL_INVDSTALPHA;
   default:
      BPE_VERIFYA( false, "Invalid blend mode specified" );
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetProjectionMatrix(CMatrix4 const &matrix, EProjectionType const projectionType ) 
{ 
   mProjectionMatrix = matrix;

   Mtx44 mtx1;

   memcpy( &mtx1, matrix.Transpose().GetPtr(), sizeof( mtx1 ) );

   Mtx44 mtx2;
   if ( projectionType == kPT_Orthogonal )
   {
      MTXOrtho( mtx2, 0, -448, 0, 640, -1, 1 );
   }

   GXSetProjection( mtx1, ( projectionType == kPT_Orthogonal ) ? GX_ORTHOGRAPHIC : GX_PERSPECTIVE );
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetPerspectiveProjection(CAngle const & fov, 
                                                  real32 const aspect, 
                                                  real32 const minClip, 
                                                  real32 const maxClip )
{
   mFrustumNear = minClip;
   mFrustumFar = maxClip;

   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(
      CMatrix4::Translation(CVector3(mViewportJitterX, mViewportJitterY, 0.0f)) * CMatrix4::PerspectiveNegZ(fov, aspect, minClip, maxClip),
      CRenderBackend::kPT_Perspective);
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetOrthographicProjection(real32 const width,
                                                   real32 const height,
                                                   real32 const zNear,
                                                   real32 const zFar)
{
   mFrustumNear = zNear;
   mFrustumFar = zFar;

   real32 const realWidth = (width < 0.0f) ? mRenderTargetWidth : width;
   real32 const realHeight = (height < 0.0f) ? mRenderTargetHeight : height;

   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicNegZ(realWidth, realHeight, zNear, zFar), CRenderBackend::kPT_Orthogonal);

}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetScreenSpaceOrtho()
{
   // 1-1 pixel mapping with 0,0 at screen top left
   real32 const offsetX = -1.0f;
   real32 const offsetY = 1.0f;
   real32 const nearZ = -1.0f;
   real32 const farZ = 1.0f;
   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicOffsetNegZ(real32(GetViewWidth()), 
      real32(-GetViewHeight()),
      offsetX,
      offsetY,
      nearZ, farZ), CRenderBackend::kPT_Orthogonal);

   SetCameraMatrix(CMatrix34::Identity());
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetUniformOrtho()
{
   real32 const width = 1.0f;
   real32 const height = -1.0f;
   real32 const offsetX = -1.0f;
   real32 const offsetY = 1.0f;
   real32 const nearZ = -1.0f;
   real32 const farZ = 1.0f;
   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicOffsetNegZ(width, 
      height,
      offsetX,
      offsetY,
      nearZ, farZ), CRenderBackend::kPT_Orthogonal);

   SetCameraMatrix(CMatrix34::Identity());
}
//----------------------------------------------------------------------------

void CRenderBackend::SetVertexDescriptor( CShaderVertexDataBinding const &pvd )
{
   GXVtxDescList descs[ GX_VA_MAX_ATTR ];
   GXVtxDescList *pWritingDesc = descs;

   for ( int i = 0; i < CShaderVertexDataBinding::kMIA_Count; ++i )
   {
      pWritingDesc->attr = GXAttr( i );
      // bool will cast to GX_NONE or GX_DIRECT
      pWritingDesc->type = GXAttrType( pvd.GetMatrixIndexAttributeUsed( CShaderVertexDataBinding::EMatrixIndexAttribute( i ) ) );
      ++pWritingDesc;
   }

   for ( int i = 0; i < kVDU_TexCoord7; ++i )
   {
      pWritingDesc->attr = GXAttr( GX_VA_POS + i );
      pWritingDesc->type = GXAttrType( pvd.GetVertexAttributeType( EVertexDataUsage( i ) ) );
      ++pWritingDesc;
   }

   pWritingDesc->attr = GX_VA_NBT;
   pWritingDesc->type = GXAttrType( pvd.GetVertexAttributeType( kVDU_NBT ) );
   ++pWritingDesc;

   pWritingDesc->attr = GX_VA_NULL;
   pWritingDesc->type = GX_NONE;

   GXSetVtxDescv( descs );
}

//----------------------------------------------------------------------------

void CRenderBackend::SetModelMatrix( CMatrix34 const &matrix )
{
   mModelMatrix = matrix;

//   mModelMatrix = CMatrix34( matrix.GetLeft(), matrix.GetUp(), matrix.GetForward(), -matrix.GetTranslation() );

   FlushPNMatrixState();
}

//----------------------------------------------------------------------------

void CRenderBackend::FlushPNMatrixState()
{
   CMatrix34 worldView = mViewMatrix * mModelMatrix;

#define using_mtx worldView

   Mtx xposed = 
   { 
      { using_mtx.Get00(), using_mtx.Get10(), using_mtx.Get20(), using_mtx.Get30() },
      { using_mtx.Get01(), using_mtx.Get11(), using_mtx.Get21(), using_mtx.Get31() },
      { using_mtx.Get02(), using_mtx.Get12(), using_mtx.Get22(), using_mtx.Get32() }
   };

   GXLoadPosMtxImm( xposed, GX_PNMTX0 );

   Mtx nrmMatrix;
   MTXInvXpose( xposed, nrmMatrix );

   GXLoadNrmMtxImm( nrmMatrix, GX_PNMTX0 );
}

//----------------------------------------------------------------------------


void CBaseRenderBackend::SetCameraMatrix(CMatrix34 const &matrix)
{
   CRenderBackend *pThis = static_cast<CRenderBackend *>( this );

   pThis->mCameraMatrix = matrix;
   // Might be able to speed this up a bit!
   pThis->mViewMatrix = /* CMatrix34::Scale(CVector3(1, 1, -1)) * */ pThis->mCameraMatrix.Inverse();

   pThis->FlushPNMatrixState();

}

//----------------------------------------------------------------------------

void CRenderBackend::SetPixelStateSlow( CRVLRenderState::SPixelState const &pixelState )
{
   GXSetBlendMode( pixelState.mType, pixelState.mSrcFactor, pixelState.mDstFactor, pixelState.mLogicOp );
   GXSetZMode( (GXBool) pixelState.mDepthCompare, pixelState.mDepthFunc, (GXBool) pixelState.mDepthUpdate );
   GXSetColorUpdate( (GXBool) pixelState.mColorUpdate );
   GXSetAlphaUpdate( (GXBool) pixelState.mAlphaUpdate );

   // Todo - Alpha compare and alpha buffer

   mPixelStateHandle = CRVLRenderState::CStateHandle::EmptyID();
}

//----------------------------------------------------------------------------

void CRenderBackend::SetPixelState( CRVLRenderState::TPixelStateHandle const &pixelState )
{
   if ( mPixelStateHandle != pixelState.GetUniqueId() )
   {
      SetPixelStateSlow( pixelState.GetObject() );
      mPixelStateHandle = pixelState.GetUniqueId();
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetVariableUnitStatesSlow( CRVLRenderState::SVariableUnitStates const &unitStates )
{
   GXSetNumTevStages( unitStates.mNumTev );
   GXSetNumIndStages( unitStates.mNumInd );
   GXSetNumTexGens( unitStates.mNumTex );
   GXSetNumChans( unitStates.mNumChan );

   mVariableUnitStatesHandle = CRVLRenderState::CStateHandle::EmptyID();
}

//----------------------------------------------------------------------------

void CRenderBackend::SetVariableUnitStates( CRVLRenderState::TVariableUnitStatesHandle const &unitStates )
{
   if ( mVariableUnitStatesHandle != unitStates.GetUniqueId() )
   {
      SetVariableUnitStatesSlow( unitStates.GetObject() );
      mVariableUnitStatesHandle = unitStates.GetUniqueId();
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTevStageSlow( GXTevStageID tevStage, CRVLRenderState::STevStageState const &state )
{
   GXSetTevColorIn( tevStage, state.mCA, state.mCB, state.mCC, state.mCD );
   GXSetTevAlphaIn( tevStage, state.mAA, state.mAB, state.mAC, state.mAD );
   GXSetTevColorOp( tevStage, state.mColorOp.mOp, state.mColorOp.mBias, state.mColorOp.mScale, state.mColorOp.mClamp, state.mColorOp.mOutReg );
   GXSetTevAlphaOp( tevStage, state.mAlphaOp.mOp, state.mAlphaOp.mBias, state.mAlphaOp.mScale, state.mAlphaOp.mClamp, state.mAlphaOp.mOutReg );
   GXSetTevOrder( tevStage, state.mTexCoord, state.mTexMap, state.mChannel );
   GXSetTevKColorSel( tevStage, state.mKColorSel );
   GXSetTevKAlphaSel( tevStage, state.mKAlphaSel );

   mTevStageStateHandle[ tevStage ] = CRVLRenderState::CStateHandle::EmptyID();
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTevStage( GXTevStageID tevStage, CRVLRenderState::TTevStageStateHandle const &state )
{
   if ( mTevStageStateHandle[ tevStage ] != state.GetUniqueId() )
   {
      SetTevStageSlow( tevStage, state.GetObject() );
      mTevStageStateHandle[ tevStage ] = state.GetUniqueId();
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTexCoordGenSlow( GXTexCoordID texCoord, CRVLRenderState::STexCoordGen const &state )
{
   GXSetTexCoordGen2( texCoord, state.mFunc, state.mSrc, state.mMtx, state.mNormalize, state.mPostMtx );
   mTexCoordGenHandle[ texCoord ] = CRVLRenderState::CStateHandle::EmptyID();
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTexCoordGen( GXTexCoordID texCoord, CRVLRenderState::TTexcoordGenHandle const &state )
{
   if ( mTexCoordGenHandle[ texCoord ] != state.GetUniqueId() )
   {
      SetTexCoordGenSlow( texCoord, state.GetObject() );
      mTexCoordGenHandle[ texCoord ] = state.GetUniqueId();
   }
}