//----------------------------------------------------------------------------
// CMayaSpline.h
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CSpline.h"
#include "Engine/Math/CAngle.h"

//----------------------------------------------------------------------------

class ENGINE_API CMayaSplineKey
{
public:
   enum ETangentType
   {
      kTangent_Fixed,
      kTangent_Linear,
      kTangent_Flat,
      kTangent_Step,
      kTangent_Slow,
      kTangent_Fast,
      kTangent_Smooth,
      kTangent_Clamped,
      kTangent_Plateau
   };

   CMayaSplineKey( real32 const time, 
                   real32 const value, 
                   ETangentType const inTangentType = kTangent_Smooth, 
                   ETangentType const outTangentType = kTangent_Smooth,
                   CAngle const & inAngle = CAngle::FromRadians(0.0f),
                   real32 const inWeight = 1.0f,
                   CAngle const & outAngle = CAngle::FromRadians(0.0f),
                   real32 const outWeight = 1.0f )
   :  mTime( time )
   ,  mValue( value )
   ,  mInTangentType( inTangentType )
   ,  mOutTangentType( outTangentType )
   ,  mInAngle( inAngle )
   ,  mInWeight( inWeight )
   ,  mOutAngle( outAngle )
   ,  mOutWeight( outWeight )
   {
   }
   
   bool const operator < ( CMayaSplineKey const & rhs ) const
   {
      return mTime < rhs.mTime;
   }
   
public:
   real32         mTime;
   real32         mValue;
   
   ETangentType   mInTangentType;
   ETangentType   mOutTangentType;
   
   CAngle         mInAngle;
   real32         mInWeight;
   CAngle         mOutAngle;
   real32         mOutWeight;
};

//----------------------------------------------------------------------------

MANAGED_PUBLIC class ENGINE_API CMayaSpline
{
public:
   CMayaSpline( std::vector<CMayaSplineKey> const & keys,
                bool const isWeighted,
                CSpline::EInfinity const preInfinity, 
                CSpline::EInfinity const postInfinity )
   :  mKeys( keys )
   ,  mIsWeighted( isWeighted )
   ,  mPreInfinity( preInfinity )
   ,  mPostInfinity( postInfinity )
   {
   }

   CSpline const BuildSpline() const;

public:
   std::vector<CMayaSplineKey>   mKeys;
   bool                          mIsWeighted;
   CSpline::EInfinity            mPreInfinity;
   CSpline::EInfinity            mPostInfinity;
};

//----------------------------------------------------------------------------

