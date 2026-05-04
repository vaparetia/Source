//----------------------------------------------------------------------------
// NAnimationBlendTree.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#ifndef SPU
#include "Engine/Math/CQuaternion.h"
#include "Engine/Math/CVector4.h"
#endif

//----------------------------------------------------------------------------

// NOTE: This structure must be allocated 16 byte aligned!
struct SAnimationLocomotionState
{
#ifdef SPU
   typedef real32       TQuaternion[4];
   typedef real32       TVector[4];
#else
   typedef CQuaternion  TQuaternion;
   typedef CVector4     TVector;
#endif

   // Total amount of rotational movement of root node during one cycle (Vectormath::Aos::Quat)
   TQuaternion mAnimationDeltaRotation;
   // Total amount of translational movement of root node during one cycle (Vectormath::Aos::Vector3, actually 4 floats)
   TVector     mAnimationDeltaTranslation;

   // Root node rotation during last animation evaluation (Vectormath::Aos::Quat)
   TQuaternion mLastEvalRotation;
   // Root node translation during last animation evaluation (Vectormath::Aos::Point3, actually 4 floats)
   TVector     mLastEvalTranslation;

   // Number of loops since last update (can be positive or negative depending on which way it looped)
   int         mLoopCount;
   
   // Flag to determine if the last eval rotation/translation is valid
   int         mLastEvalValid;
   
   // padding to make structure 16 byte aligned.
   int         mPad[2];
};