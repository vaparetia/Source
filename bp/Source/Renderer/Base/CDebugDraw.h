//----------------------------------------------------------------------------
// CDebugDraw.h
// Copyright 2004
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Graphics/CColor.h"

//----------------------------------------------------------------------------

class CAABox;
class CMatrix34;
class CD3DRenderer;

//----------------------------------------------------------------------------

class CDebugDraw
{
public:
   enum EDrawType
   {
      kDT_ZCompareEnabled,
      kDT_ZCompareDisabled,

      kDT_Count
   };

   struct SVert
   {
      SVert( CVector3 const & pos,
             CColor const & color,
             int const numFramesToRender)
      : mPos( pos )
      , mColor( color )
      , mRenderFrames(numFramesToRender)
      {
      }

      CVector3 mPos;
      CColor   mColor;
      int      mRenderFrames;
   };

   struct SDebugText
   {
      SDebugText( char const * const pText,
                  CVector3 const & pos,
                  real32 const scale,
                  CColor const & color,
                  EDrawType const drawType,
                  int const numFramesToRender )
         : mText(pText)
         , mPos(pos)
         , mScale(scale)
         , mColor(color)
         , mDrawType(drawType)
         , mRenderFrames(numFramesToRender)
      {
      }

      std::string mText;
      CVector3    mPos;
      real32      mScale;
      CColor      mColor;
      EDrawType   mDrawType;
      int         mRenderFrames;
   };
   
   enum EUpdateType
   {
      kUT_GamePaused,
      kUT_GameActive,
      kUT_Count
   };

public:
   RENDERER_API CDebugDraw();

   RENDERER_API void Clear();
   RENDERER_API void Update(EUpdateType const updateType);

   RENDERER_API void Render();

   RENDERER_API void AddLine( CVector3 const & start,
                 CVector3 const & end,
                 CColor const & color = CColor::White(),
                 EDrawType const drawType = kDT_ZCompareEnabled,
                 int const numFramesToRender = 1);

   RENDERER_API void AddBox(  CAABox const & box,
                              CColor const & color = CColor::White(),
                              EDrawType const drawType = kDT_ZCompareEnabled,
                              int const numFramesToRender = 1);

   RENDERER_API void AddBox(  CAABox const & box,
                              CMatrix34 const &transform,
                              CColor const & color = CColor::White(),
                              EDrawType const drawType = kDT_ZCompareEnabled,
                              int const numFramesToRender = 1);

   RENDERER_API void AddCube( real32 const size,
                              CMatrix34 const &transform,
                              CColor const & color = CColor::White(),
                              EDrawType const drawType = kDT_ZCompareEnabled,
                              int const numFramesToRender = 1);


   RENDERER_API void AddSphere(  real32 const radius,
                                 int32 const numSubsLat,
                                 int32 const numSubsLong,
                                 CMatrix34 const &transform,
                                 CColor const & color = CColor::White(),
                                 EDrawType const drawType = kDT_ZCompareEnabled,
                                 int const numFramesToRender = 1);


   RENDERER_API void AddAxis( CMatrix34 const & transform,
                              real32 const scale = 1.0f,
                              CColor const & xAxisColor = CColor::Red(),
                              CColor const & yAxisColor = CColor::Green(),
                              CColor const & zAxisColor = CColor::Blue(),
                              EDrawType const drawType = kDT_ZCompareEnabled,
                              int const numFramesToRender = 1);


   RENDERER_API void AddGrid( int extents,
                              CColor const & majorLineColor = CColor::White(),
                              CColor const & minorLineColor = CColor::Grey(),
                              CColor const & centerAxisColor = CColor::Yellow());

   // Drawn -1 -> 1, z-axis normal
   RENDERER_API void AddPlane(CMatrix34 const &transform,
                              CColor const & color = CColor::White(),
                              EDrawType const drawType = kDT_ZCompareEnabled,
                              int const numFramesToRender = 1);


   RENDERER_API void AddCylinder(CVector3 const & start,
                                 CVector3 const & unitDirection,
                                 real32 const length,
                                 real32 const radius,
                                 int32 const numCylinderSubs,
                                 int32 const numCircleSubs,
                                 CColor const & color = CColor::White(),
                                 EDrawType const drawType = kDT_ZCompareEnabled,
                                 int const numFramesToRender = 1);


   RENDERER_API void AddCylinder(CVector3 const & start,
                                 CVector3 const & unitDirection,
                                 real32 const length,
                                 real32 const radius1,
                                 real32 const radius2,
                                 int32 const numCylinderSubs,
                                 int32 const numCircleSubs,
                                 CColor const & color = CColor::White(),
                                 EDrawType const drawType = kDT_ZCompareEnabled,
                                 int const numFramesToRender = 1);


   RENDERER_API void AddCapsule(real32 const length,
                                real32 const radius,
                                int32 const numCylinderSubs,
                                int32 const numCircleSubs,
                                CMatrix34 const & transform,
                                CColor const & color = CColor::White(),
                                EDrawType const drawType = kDT_ZCompareEnabled,
                                int const numFramesToRender = 1);


   RENDERER_API void AddCircle(CVector3 const & center, 
                               CVector3 const & direction, 
                               real32 const radius, 
                               int const subs, 
                               CColor const & color, 
                               EDrawType const drawType = kDT_ZCompareEnabled,
                               int const numFramesToRender = 1);

   RENDERER_API void AddAxisRings(CMatrix34 const &transform,
                                  real32 const radius,
                                  EDrawType const drawType = kDT_ZCompareEnabled,
                                  int const numFramesToRender = 1);

   RENDERER_API void AddArc(CMatrix34 const &transform,
                            real32 const minAngle,
                            real32 const maxAngle,
                            real32 const radius, 
                            int const subs, 
                            CColor const & color, 
                            EDrawType const drawType = kDT_ZCompareEnabled,
                            int const numFramesToRender = 1);

   RENDERER_API void AddText( char const * const pText,
                              CVector3 const & pos,
                              real32 const scale = 1.0f,
                              CColor const &color = CColor::White(),
                              EDrawType const drawType = kDT_ZCompareEnabled,
                              int const numFramesToRender = 1);

private:
   std::vector<SVert>   mLineListVertsZCompareEnabled;
   std::vector<SVert>   mLineListVertsZCompareDisabled;

   // Single frame versions for performance, saves on iterating over all debug lines for expiration.
   std::vector<SVert>   mLineListVertsZCompareEnabled_SingleFrame;
   std::vector<SVert>   mLineListVertsZCompareDisabled_SingleFrame;

   std::vector<SDebugText> mTextList;
};

//----------------------------------------------------------------------------

extern RENDERER_API CDebugDraw* gpDebugDraw;

//----------------------------------------------------------------------------



