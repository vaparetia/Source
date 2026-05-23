//----------------------------------------------------------------------------
// CSpline.h
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

class CInputStream;
class COutputStream;
class CLinearCopyStream;

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector2.h"

//----------------------------------------------------------------------------

MANAGED_PUBLIC class ENGINE_API CSplineKey
{
public:

   // no stream support on SPU
#if !defined(SPU)
   explicit CSplineKey(CInputStream & stream)
   :  mTime( stream.ReadReal32() )
   ,  mValue( stream.ReadReal32() )
   ,  mInTangent( stream )
   ,  mOutTangent( stream )
   {
   }
   
   void PutTo(COutputStream & stream) const
   {
      stream.WriteReal32(mTime);
      stream.WriteReal32(mValue);
      stream.Put(mInTangent);
      stream.Put(mOutTangent);
   }
#endif

   CSplineKey( real32 const time, real32 const value, CVector2 const & inTangent, CVector2 const & outTangent )
      :  mTime( time )
      ,  mValue( value )
      ,  mInTangent( inTangent )
      ,  mOutTangent( outTangent )
   {
   }

   bool const operator < ( CSplineKey const & rhs ) const
   {
      return mTime < rhs.mTime;
   }

   real32   mTime;
   real32   mValue;
   CVector2 mInTangent;
   CVector2 mOutTangent;
};

//----------------------------------------------------------------------------

MANAGED_PUBLIC struct SSplineEvaluationCache
{
   SSplineEvaluationCache()
      :  mLastIndex( -1 )
      ,  mLastInterval( -1 )
      ,  mIsStep( false )
      ,  mIsLinear( false )
      ,  mX1( 0.0f )
      ,  mX4( 0.0f )
   {
   }

   int   mLastIndex;
   int   mLastInterval;

   bool  mIsStep;
   bool  mIsLinear;

   real32 mX1; // start x of segment
   real32 mX4; // end x of segment
   real32 mCoeff[4]; // bezier x parameters (only used for weighted curves)
   real32 mPolyY[4]; // bezier y parameters
};

//----------------------------------------------------------------------------

MANAGED_PUBLIC class ENGINE_API CSpline
{
public:
   enum EInfinity
   {
      kInfinity_Constant,
      kInfinity_Linear,
      kInfinity_Cycle,
      kInfinity_CycleRelative,
      kInfinity_Oscillate
   };

public:
   explicit CSpline(CInputStream & stream);

   explicit CSpline(bool const isWeighted = false,
                    EInfinity const preInfinity = kInfinity_Constant,
                    EInfinity const postInfinity = kInfinity_Constant );
   ~CSpline();

   void ConstructFromStream( CInputStream &stream);
   void PutTo(COutputStream & stream) const;
   void CopyToLinearStream(CLinearCopyStream &stream) const;
   void CopyToLinearStream_Inplace(CLinearCopyStream &stream, int bufferPos) const;
   void AddKey( CSplineKey const & key );
   
   real32 const Evaluate( real32 const time, SSplineEvaluationCache * const pCache ) const;

private:
   real32 const EvaluateInfinities(real32 const time, bool evalPre, SSplineEvaluationCache * const pCache) const;

   void BezierCreate(SSplineEvaluationCache * const pCache, real32 x[4], real32 y[4]) const;
   real32 const BezierEvaluate(SSplineEvaluationCache * const pCache, real32 const time) const;
   
   void HermiteCreate(SSplineEvaluationCache * const pCache, real32 x[4], real32 y[4] ) const;
   real32 const HermiteEvaluate(SSplineEvaluationCache * const pCache, real32 const time) const;

   bool FindKey(real32 time, int *index) const;

   void EnsureKeysSorted() const;
   
public:
   EInfinity   mPreInfinity;
   EInfinity   mPostInfinity;
   bool        mIsWeighted;
   //bool        mIsStatic;

   bool mKeysSorted;

   // Vector changes size based on debug modes, add padding so develop SPU code matches debug
#if (_HAS_ITERATOR_DEBUGGING == 0) || !defined(_HAS_ITERATOR_DEBUGGING)
   uint32   mPaddingForSPU;
#endif
   bpe::vector_s<CSplineKey> mKeys;
};

//----------------------------------------------------------------------------
