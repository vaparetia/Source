//------------------------------------------------------------------------------------------
// CDebugDraw.cpp
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CDebugDraw.h"

//------------------------------------------------------------------------------------------

#include "Engine/Primitives/CAABox.h"
#include "Engine/Math/CVector3.h"

#include "Renderer/Base/Backend/CRenderBackend.h"

#if BPE_TARGET == BPE_TARGET_WIN32   
#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"
#endif

//------------------------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

CDebugDraw * gpDebugDraw = NULL;

//------------------------------------------------------------------------------------------

CDebugDraw::CDebugDraw()
{
   gpDebugDraw = this;
}

//------------------------------------------------------------------------------------------

void CDebugDraw::Clear()
{
   mLineListVertsZCompareEnabled.clear();
   mLineListVertsZCompareDisabled.clear();
   mLineListVertsZCompareEnabled_SingleFrame.clear();
   mLineListVertsZCompareDisabled_SingleFrame.clear();

}

//------------------------------------------------------------------------------------------

void CDebugDraw::Update(EUpdateType const updateType)
{
   // Remove all single frame debug line entries as this is the common case.
   mLineListVertsZCompareEnabled_SingleFrame.clear();
   mLineListVertsZCompareDisabled_SingleFrame.clear();
   
   if (updateType == kUT_GamePaused)
   {
      // Don't remove timed lines.
      return;
   }

   // Iterate over other draw arrays and remove expired debug lines.
   for (int loop = 0; loop < 2; loop++)
   {
      std::vector<SVert> &verts = (loop == 0) ? mLineListVertsZCompareEnabled : mLineListVertsZCompareDisabled;
      for (std::vector<SVert>::iterator it = verts.begin(); it != verts.end();)
      {
         SVert &vert = *it;
         vert.mRenderFrames--;
         if (vert.mRenderFrames <= 0)
         {
            // Erase, last to current position for performance.
            if ((it + 1) != verts.end())
            {
               *it = verts.back();
               verts.pop_back();
            }
            else
            {
               verts.pop_back();
               it = verts.end();
            }
         }
         else
         {
            ++it;
         }
      }
   }

   std::vector<SDebugText>::iterator it = mTextList.begin();
   while (it != mTextList.end())
   {
      SDebugText &text = *it;
      if (--text.mRenderFrames <= 0)
      {
         // Erase, last to current position for performance.
         if ((it + 1) != mTextList.end())
         {
            *it = mTextList.back();
            mTextList.pop_back();
         }
         else
         {
            mTextList.pop_back();
            it = mTextList.end();
         }
      }
      else
      {
         ++it;
      }
   }
}

//------------------------------------------------------------------------------------------

void CDebugDraw::Render()
{
   CRenderBackend & renderer = *RenderBackend();

#pragma BPE_TODOMSG("Rework debug line rendering to work cross platform.")

#if BPE_TARGET == BPE_TARGET_WIN32

   if ( !mLineListVertsZCompareEnabled.empty() ||
      !mLineListVertsZCompareDisabled.empty() ||
      !mLineListVertsZCompareEnabled_SingleFrame.empty() ||
      !mLineListVertsZCompareDisabled_SingleFrame.empty() )
   {
      IDirect3DDevice9 *d3dDevice = D3DDeviceUncached();

      // model view transform
      {
         CMatrix4 const world = CMatrix4::Identity();
         d3dDevice->SetTransform(D3DTS_WORLD, reinterpret_cast<const D3DMATRIX*>(&world));
      }

      // view port transform
      {
         CMatrix4 const & currentTransform = RenderBackend()->GetViewMatrix();
         d3dDevice->SetTransform(D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(&currentTransform));
      }

      // set perspective transform
      {
         CMatrix4 const & currentTransform = RenderBackend()->GetProjectionMatrix();
         d3dDevice->SetTransform(D3DTS_PROJECTION, reinterpret_cast<const D3DMATRIX*>(&currentTransform));
      }

      d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
      d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      
      d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
      d3dDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE);

      d3dDevice->SetVertexShader(NULL);
      d3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);

      d3dDevice->SetPixelShader(NULL);
      d3dDevice->SetTexture(0, NULL);

      d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
      d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA);

      d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

      if( !mLineListVertsZCompareEnabled.empty() )
      {      
         d3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 
                                      mLineListVertsZCompareEnabled.size() / 2, 
                                      &mLineListVertsZCompareEnabled[0], 
                                      sizeof(SVert));
      }

      if( !mLineListVertsZCompareEnabled_SingleFrame.empty() )
      {      
         d3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 
                                      mLineListVertsZCompareEnabled_SingleFrame.size() / 2, 
                                      &mLineListVertsZCompareEnabled_SingleFrame[0], 
                                      sizeof(SVert));
      }

      d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

      if( !mLineListVertsZCompareDisabled.empty() )
      {
         d3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 
                                      mLineListVertsZCompareDisabled.size() / 2, 
                                      &mLineListVertsZCompareDisabled[0], 
                                      sizeof(SVert));
      }

      if( !mLineListVertsZCompareDisabled_SingleFrame.empty() )
      {
         d3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 
                                      mLineListVertsZCompareDisabled_SingleFrame.size() / 2, 
                                      &mLineListVertsZCompareDisabled_SingleFrame[0], 
                                      sizeof(SVert));
      }

      d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
      d3dDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);
      d3dDevice->SetFVF(NULL);

      //renderer.GetModelViewMatrixStack().PopMatrix();
      
      gpRenderBackend->ForceVertexDataRebind();
   }
#elif BPE_TARGET==BPE_TARGET_RVL

   // 1 TEV, constant color
   CRVLRenderState::SPixelState pixelState(
      GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR,
      CRVLRenderState::SPixelState::kDepthCompare_Enabled, GX_LEQUAL, CRVLRenderState::SPixelState::kDepthUpdate_Disabled,
      CRVLRenderState::SPixelState::kColorUpdate_Enabled, CRVLRenderState::SPixelState::kAlphaUpdate_Disabled, CRVLRenderState::SPixelState::kDstAlpha_FromTev,
      GX_ALWAYS );

   gpRenderBackend->SetPixelStateSlow( pixelState );
   gpRenderBackend->SetTevStageSlow( GX_TEVSTAGE0, CRVLRenderState::STevStageState(
      GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST, GX_TEV_KCSEL_K0,
      GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST, GX_TEV_KASEL_1,
      CRVLRenderState::GetDefaultTevOp(), CRVLRenderState::GetDefaultTevOp(),
      GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL ) );
   gpRenderBackend->SetVariableUnitStates( CRVLRenderState::GetUnitStatesForSameTevsAndTexGensNoChan( 1 ) );

   for ( int i = 0; i < mLineListVertsZCompareEnabled.size(); i += 2 )
   {
      SVert const &vert1 = mLineListVertsZCompareEnabled[i];
      SVert const &vert2 = mLineListVertsZCompareEnabled[i+1];

      gpRenderBackend->RenderLine3d( vert1.mPos, vert2.mPos, vert1.mColor );
   }

   for ( int i = 0; i < mLineListVertsZCompareEnabled_SingleFrame.size(); i += 2 )
   {
      SVert const &vert1 = mLineListVertsZCompareEnabled_SingleFrame[i];
      SVert const &vert2 = mLineListVertsZCompareEnabled_SingleFrame[i+1];

      gpRenderBackend->RenderLine3d( vert1.mPos, vert2.mPos, vert1.mColor );
   }

   pixelState.mDepthCompare = CRVLRenderState::SPixelState::kDepthCompare_Disabled;
   gpRenderBackend->SetPixelStateSlow( pixelState );

   for ( int i = 0; i < mLineListVertsZCompareDisabled.size(); i += 2 )
   {
      SVert const &vert1 = mLineListVertsZCompareDisabled[i];
      SVert const &vert2 = mLineListVertsZCompareDisabled[i+1];

      gpRenderBackend->RenderLine3d( vert1.mPos, vert2.mPos, vert1.mColor );
   }

   for ( int i = 0; i < mLineListVertsZCompareDisabled_SingleFrame.size(); i += 2 )
   {
      SVert const &vert1 = mLineListVertsZCompareDisabled_SingleFrame[i];
      SVert const &vert2 = mLineListVertsZCompareDisabled_SingleFrame[i+1];

      gpRenderBackend->RenderLine3d( vert1.mPos, vert2.mPos, vert1.mColor );
   }

#endif

   if (!mTextList.empty())
   {
      CMatrix4 const & currentTransform = renderer.GetViewMatrix();
      CMatrix4 const & projectionMatrix = renderer.GetProjectionMatrix();
      CMatrix4 const worldToScreen = projectionMatrix*currentTransform;
      std::vector<SDebugText>::iterator it = mTextList.begin();
      while (it != mTextList.end())
      {
         SDebugText &text = *it++;
         CVector3 pos = worldToScreen*text.mPos;
         if (pos.mZ < 1.0f)
         {
            pos.mX = renderer.GetViewWidth()*(0.5f*(pos.mX + 1.0f));
            pos.mY = renderer.GetViewHeight()*(0.5f*(1.0f - pos.mY));
            renderer.RenderText(text.mText.c_str(), pos.mX, pos.mY, kRenderFonts_Console, text.mColor, 0);
         }
         
      }
   }
}

//------------------------------------------------------------------------------------------

void CDebugDraw::AddLine( CVector3 const & start,
                          CVector3 const & end,
                          CColor const & color,
                          EDrawType const drawType,
                          int const numFramesToRender)
{
   std::vector<SVert> * pArray = NULL;
   // Optimization for large amounts of single frame lines.
   if (numFramesToRender == 1)
   {
      // Add to 'quick erase' arrays
      pArray = (drawType == kDT_ZCompareEnabled) ? &mLineListVertsZCompareEnabled_SingleFrame : &mLineListVertsZCompareDisabled_SingleFrame;
   }
   else
   {
      // Add to 'iterate and check' arrays
      pArray = (drawType == kDT_ZCompareEnabled) ? &mLineListVertsZCompareEnabled : &mLineListVertsZCompareDisabled;
   }

   pArray->push_back( SVert( start, color, numFramesToRender ) );
   pArray->push_back( SVert( end, color, numFramesToRender ) );
}

//------------------------------------------------------------------------------------------

void CDebugDraw::AddBox( CAABox const & box,
                         CColor const & color,
                         EDrawType const drawType,
                         int const numFramesToRender)
{
   reserved_vector<CVector3, 8> boxPoints;
   
   for( int i = 0; i < 8; ++i )
   {
      boxPoints.push_back( box.GetPoint( i ) );
   }

   reserved_vector<uint16, 24> const indices = box.GetLineLoopIndices();

   for( uint16 i = 0; i < indices.size(); i += 2 )
   {
      AddLine( boxPoints[ indices[i] ],
               boxPoints[ indices[i + 1] ],
               color,
               drawType, numFramesToRender );
   }
}

//------------------------------------------------------------------------------------------

void CDebugDraw::AddBox( CAABox const & box,
                         CMatrix34 const &transform,
                         CColor const & color,
                         EDrawType const drawType,
                         int const numFramesToRender)
{
   {
      reserved_vector<CVector3, 8> boxPoints;
      
      for( int i = 0; i < 8; ++i )
      {
         boxPoints.push_back( box.GetPoint( i ) );
      }

      reserved_vector<uint16, 24> const indices = box.GetLineLoopIndices();

      for( uint16 i = 0; i < indices.size(); i += 2 )
      {
         AddLine( transform * boxPoints[ indices[i] ],
                  transform * boxPoints[ indices[i + 1] ] ,
                  color,
                  drawType, numFramesToRender );
      }
   }
}

//------------------------------------------------------------------------------------------

void CDebugDraw::AddCube(  real32 const size,
                           CMatrix34 const &transform,
                           CColor const & color,
                           EDrawType const drawType,
                           int const numFramesToRender)
{
   real32 const halfSize = size * 0.5f;
   CAABox const box(CAABox::MakeBoxCenteredAt(CVector3::Zero(), halfSize));
   AddBox(box, transform, color, drawType, numFramesToRender);
}

//------------------------------------------------------------------------------------------

void CDebugDraw::AddSphere(real32 const radius,
                           int32 const numSubsLat,
                           int32 const numSubsLong,
                           CMatrix34 const &transform,
                           CColor const & color,
                           EDrawType const drawType,
                           int const numFramesToRender)                           
{

   real32 const latInterval = (gkPi32 * 2.0f) / numSubsLat;
   real32 const longInterval = (gkPi32 * 2.0f) / numSubsLong;
   for (int latLoop = 0; latLoop < numSubsLat; latLoop++)
   {
      real32 const lat = latLoop * latInterval;
      real32 const nextLat = lat + latInterval;

      real32 const cosLat = radius * cosf( lat );
      real32 const sinLat = radius * sinf( lat );

      real32 const nextCosLat = radius * cosf( nextLat );
      real32 const nextSinLat = radius * sinf( nextLat );

      real32 lon = -gkPiOverTwo32;

      for (int longLoop = 0; longLoop < numSubsLong; longLoop++)
      {
         real32 const nextLon = lon + longInterval;

         real32 const cosLon = cosf( lon );
         real32 const sinLon = sinf( lon );

         real32 const nextCosLon = cosf( nextLon );
         real32 const nextSinLon = sinf( nextLon );
         lon = nextLon;

         CVector3 v1( cosLat * cosLon, cosLat * sinLon, sinLat );
         CVector3 v2( cosLat * nextCosLon, cosLat * nextSinLon, sinLat );
         CVector3 v3( nextCosLat * nextCosLon, nextCosLat * nextSinLon, nextSinLat );
         v1 = transform * v1;
         v2 = transform * v2;
         v3 = transform * v3;

         AddLine( v1, v2, color, drawType, numFramesToRender );
         AddLine( v2, v3, color, drawType, numFramesToRender );
      }
   }
}

//------------------------------------------------------------------------------------------

void CDebugDraw::AddAxis( CMatrix34 const & transform,
                          real32 const scale,
                          CColor const & xAxisColor,
                          CColor const & yAxisColor,
                          CColor const & zAxisColor,
                          EDrawType const drawType,
                          int const numFramesToRender)
{
   CVector3 const center = transform.GetTranslation();
   
   AddLine( center, 
            center + transform.GetLeft() * scale,
            xAxisColor,
            drawType, numFramesToRender );

   AddLine( center, 
            center + transform.GetUp() * scale,
            yAxisColor,
            drawType, numFramesToRender );

   AddLine( center, 
            center + transform.GetForward() * scale,
            zAxisColor,
            drawType, numFramesToRender );

}

//----------------------------------------------------------------------------

void CDebugDraw::AddGrid( int extents,
                          CColor const & majorLineColor,
                          CColor const & minorLineColor,
                          CColor const & centerAxisColor )
{
   for( int i = -extents; i <= extents; ++i )
   {
      CColor curLineColor = (i == 0) ? centerAxisColor 
                                     : ((i % 10) == 0 ) ? majorLineColor
                                                        : minorLineColor;

      AddLine( CVector3((real32)i, 0, (real32)-extents), CVector3((real32)i, 0, (real32)extents), curLineColor );
      AddLine( CVector3((real32)-extents, 0, (real32)i), CVector3((real32)extents, 0, (real32)i), curLineColor );
   }

   AddAxis(CMatrix34::Identity(), 2.0f, CColor::Red(), CColor::Green(), CColor::Blue());
}

//----------------------------------------------------------------------------

void CDebugDraw::AddPlane(CMatrix34 const &transform,
                          CColor const & color,
                          EDrawType const drawType,
                          int const numFramesToRender)
{
   AddLine( transform * CVector3::Zero(), 
            transform * CVector3::ZAxis(),
            color,
            drawType, numFramesToRender );

   AddLine( transform * CVector3(1,  1, 0),
            transform * CVector3(1, -1, 0),
            color,
            drawType, numFramesToRender );

   AddLine( transform * CVector3( 1, -1, 0),
            transform * CVector3(-1, -1, 0),
            color,
            drawType, numFramesToRender );

   AddLine( transform * CVector3(-1, -1, 0),
            transform * CVector3(-1,  1, 0),
            color,
            drawType, numFramesToRender );

   AddLine( transform * CVector3(-1,  1, 0),
            transform * CVector3( 1,  1, 0),
            color,
            drawType, numFramesToRender );
}

//----------------------------------------------------------------------------

void CDebugDraw::AddCylinder(CVector3 const & start,
                             CVector3 const & unitDirection,
                             real32 const length,
                             real32 const radius,
                             int32 const numCylinderSubs,
                             int32 const numCircleSubs,
                             CColor const & color,
                             EDrawType const drawType,
                             int const numFramesToRender)
{
   AddCylinder(start, unitDirection, length, radius, radius, numCylinderSubs, numCircleSubs, color, drawType, numFramesToRender);
}

//----------------------------------------------------------------------------

void CDebugDraw::AddCylinder(CVector3 const & start,
                             CVector3 const & unitDirection,
                             real32 const length,
                             real32 const radius1,
                             real32 const radius2,
                             int32 const numCylinderSubs,
                             int32 const numCircleSubs,
                             CColor const & color,
                             EDrawType const drawType,
                             int const numFramesToRender)
{
   for( int i = 0; i < numCylinderSubs; ++i )
   {
      real32 const factor = static_cast<real32>(i) / numCylinderSubs;
      
      CVector3 const currentCenterPos = start + unitDirection * length * factor;
      real32 const currentRadius = radius1 + (radius2 - radius1) * factor;

      AddCircle(currentCenterPos, unitDirection, currentRadius, numCircleSubs, color, drawType, numFramesToRender);
   }
}

//----------------------------------------------------------------------------

void CDebugDraw::AddCapsule(real32 const length,
                            real32 const radius,
                            int32 const numCylinderSubs,
                            int32 const numCircleSubs,
                            CMatrix34 const & transform,
                            CColor const & color,
                            EDrawType const drawType,
                            int const numFramesToRender)
{
   // Temp: Draw with sphere/cylinder for now
   // Cylinder draw starts at initial translation, compensate
   CVector3 const up(transform.GetUp());
   CVector3 const translation(transform.GetTranslation() + up * -0.5f * length);
   AddCylinder(translation, up, length, radius, radius, numCylinderSubs, numCircleSubs, color, drawType, numFramesToRender);

   CMatrix34 sphereTransform(transform);
   // Sphere already offset
   sphereTransform.SetTranslation(translation);
   AddSphere(radius, numCircleSubs, numCircleSubs, sphereTransform, color, drawType, numFramesToRender);
   // Double offset for second sphere
   sphereTransform.SetTranslation(translation + up * length);
   AddSphere(radius, numCircleSubs, numCircleSubs, sphereTransform, color, drawType, numFramesToRender);
}

//----------------------------------------------------------------------------

void CDebugDraw::AddCircle(CVector3 const & center,
                           CVector3 const & direction, 
                           real32 const radius, 
                           int const subs, 
                           CColor const & color,
                           EDrawType const drawType,
                           int const numFramesToRender)
{
   CMatrix34 const lookAt = CMatrix34::LookAt(center, center + direction);

   real32 const interval = (gkPi32 * 2.0f) / subs;

   real32 current = 0.0f;
   real32 currentX = radius;
   real32 currentY = 0.0f;

   for( int i = 0; i < subs; ++i )
   {
      current += interval;
      real32 const nextX = radius * cosf(current);
      real32 const nextY = radius * sinf(current);

      CVector3 const v1 = lookAt * CVector3(currentX, currentY, 0.0f);
      CVector3 const v2 = lookAt * CVector3(nextX, nextY, 0.0f);
      
      AddLine(v1, v2, color, drawType, numFramesToRender);

      currentX = nextX;
      currentY = nextY;

   }
}

//----------------------------------------------------------------------------

void CDebugDraw::AddAxisRings(CMatrix34 const &transform, real32 const radius, EDrawType const drawType /* = kDT_ZCompareEnabled */, int const numFramesToRender /* = 1 */)
{
   CVector3 const center = transform.GetTranslation();
   AddCircle(center, transform.GetLeft(),    radius, 16, CColor::Red(),   drawType, numFramesToRender);
   AddCircle(center, transform.GetUp(),      radius, 16, CColor::Green(), drawType, numFramesToRender);
   AddCircle(center, transform.GetForward(), radius, 16, CColor::Blue(),  drawType, numFramesToRender);
}

//----------------------------------------------------------------------------

void CDebugDraw::AddArc(CMatrix34 const &transform,
                        real32 const minAngle,
                        real32 const maxAngle,
                        real32 const radius, 
                        int const subs, 
                        CColor const & color, 
                        EDrawType const drawType,
                        int const numFramesToRender)
{
   real32 const interval = (maxAngle - minAngle) / subs;

   real32 current = minAngle;
   real32 currentX = radius * cosf(current);
   real32 currentY = radius * sinf(current);

   for( int i = 0; i < subs; ++i )
   {
        current += interval;

        real32 const nextX = radius * cosf(current);
        real32 const nextY = radius * sinf(current);

        CVector3 const v1 = transform * CVector3(currentX, currentY, 0.0f);
        CVector3 const v2 = transform * CVector3(nextX, nextY, 0.0f);

        AddLine(v1, v2, color, drawType, numFramesToRender);

        currentX = nextX;
        currentY = nextY;
   }
}

//----------------------------------------------------------------------------

void CDebugDraw::AddText( char const * const pText,
                          CVector3 const & pos,
                          real32 const scale,
                          CColor const &color,
                          EDrawType const drawType,
                          int const numFramesToRender)
{
   mTextList.push_back(SDebugText(pText, pos, scale, color, drawType, numFramesToRender));
}