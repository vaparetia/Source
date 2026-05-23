//------------------------------------------------------------------------------------------
// MathUtils.h
//
// Catch-all for misc math functions.
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/Math/CVector4.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CMatrix34.h"

#include <math.h>

class CVector2;

//------------------------------------------------------------------------------------------

namespace MathUtils
{
#if defined(_PS3)
#if defined(SPU)

   BPE_FORCEINLINE ENGINE_API real32 const FastInvSqrt(real32 x)
   {
      // NOTE: Using frsqrte gives jittery particle sizes.
      return 1.0f / sqrtf(x);
   }

#else

   BPE_FORCEINLINE ENGINE_API real32 const FastInvSqrt(real32 x)
   {
      // NOTE: Using frsqrte gives jittery particle sizes.
#ifdef __SNC__
      return 1.0f / __builtin_fsqrts(x);
#else
      real32 r;
      __asm__("fsqrts %0,%1" : "=f"(r) : "f"(x));
      return 1.0f / r;
#endif
   }

#endif 
#else
   BPE_FORCEINLINE ENGINE_API real32 const FastInvSqrt(real32 x)
   {
      // See http://www.math.purdue.edu/~clomont/Math/Papers/2003/InvSqrt.pdf
      real32 xhalf = 0.5f * x;
      int i = *(int*)&x;               // get bits for floating value
      i = 0x5f375a86 - (i>>1);         // gives initial guess y0
      x = *(real32*)&i;                // convert bits back to float
      x = x * (1.5f - xhalf * x * x);  // Newton step, repeating increases accuracy
      return x;
   }
#endif
   
#if defined(_PS3)
   // FSel
   // Selects "a" if "condition" is >= 0
   // Selects "b" if "condition" is < 0 or NAN
   inline real32 const FSel(real32 const condition, real32 const a, real32 const b)
   {
#ifdef __SNC__
      return __builtin_fsels(condition, a, b);
#else
      real32 result;
      __asm__ ( "fsel %0, %1, %2, %3": "=f"(result): "f"(condition), "f"(a), "f"(b) );
      return result;
#endif
   }
#else
   // FSel
   // Selects "a" if "condition" is >= 0
   // Selects "b" if "condition" is < 0 or NAN
   inline real32 const FSel(real32 const condition, real32 const a, real32 const b)
   {
      return condition >= 0.0f ? a : b;
   }
#endif

   inline ENGINE_API real32 const Squared( real32 const input ) { return input * input; }
   inline ENGINE_API real32 const ClampMinMax(real32 const input, real32 const min, real32 const max)    { return bpe::max_val(min, bpe::min_val(max, input)); };
   inline ENGINE_API int32 const  ClampMinMax(int32 const input, int32 const min, int32 const max)       { return bpe::max_val(min, bpe::min_val(max, input)); };
   inline ENGINE_API real32 const ClampZeroOne(real32 const input)                                       { return bpe::max_val(0.0f, bpe::min_val(1.0f, input)); };
   inline ENGINE_API real32 const Sign(real32 const input)    { return (input < 0.0f) ? -1.0f : 1.0f; };
   inline ENGINE_API bool         SignCompare(real32 const a, real32 const b)    { return Sign(a) != Sign(b); };
   /// lerp is 0-1
   inline ENGINE_API real32 const Lerp(real32 const lerp, real32 const start, real32 const end)    { return start + (lerp * (end - start) ); };
   /// See http://www.fundza.com/rman_shaders/smoothstep/ for info about smoothstep
   /// Calculates a smooth spline interpolation between 0-1
   inline ENGINE_API real32 const SmoothStep(real32 const input)    { return (input * input * (3 - 2 * input)); };
   /// Calculates a smooth spline 0-1 interpolation between start and end
   inline ENGINE_API real32 const SmoothStep(real32 input, real32 const start, real32 const end)
   {
      input = (input - start) / (end - start);
      input = ClampZeroOne(input);
      return SmoothStep(input);
   }

   /// Calculates a smooth spline interpolation between start and end, lerp is 0-1
   inline ENGINE_API real32 const SmoothLerp(real32 const lerp, real32 const start, real32 const end)    { return start + (SmoothStep(lerp) * (end - start) ); };

   // 1D Hermite spline
   ENGINE_API real32    HermiteSpline1(real32 const p0, real32 const m0, real32 const p1, real32 const m1, real32 const t);

   // 3D Hermite spline
   ENGINE_API CVector3 const HermiteSpline3(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t);
   ENGINE_API CVector3 const HermiteSpline3Velocity(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t);
   inline ENGINE_API real32 HermiteSpline3Speed(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t) { return HermiteSpline3Velocity(p0, m0, p1, m1, t).GetLength(); }
   ENGINE_API CVector3 const HermiteSpline3Acceleration(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t);

   // Gets the arc length of a 3d Hermite spline by approximation
   ENGINE_API real32 HermiteSpline3_ArcLength(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t0, real32 const t1);
   
   // Gets the length of a 3d Hermite spline by approximation
   ENGINE_API real32 HermiteSpline3_Length(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, int const numSteps = 10);
   
   // Gets the parametric delta of a 3d Hermite spline by approximation
   ENGINE_API real32 HermiteSpline3_ParametricDelta(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const distance, real32 const startT, real32 &remainingDistance, int const numSteps = 10);

   // 1D catmull rom spline
   ENGINE_API real32    CatmullRomSpline1(real32 v0, real32 v1, real32 v2, real32 v3, real32 s);

   // 3d catmull rom spline
   ENGINE_API CVector3 const CatmullRomSpline3(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t);
   ENGINE_API CVector3 const CatmullRomSpline3(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t);
   ENGINE_API CVector3 const CatmullRomSpline3Velocity(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t);
   ENGINE_API CVector3 const CatmullRomSpline3Velocity(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t);
   inline ENGINE_API real32 CatmullRomSpline3Speed(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t) { return CatmullRomSpline3Velocity(v0, v1, v2, v3, t).GetLength(); }
   inline ENGINE_API real32 CatmullRomSpline3Speed(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t) { return CatmullRomSpline3Velocity(pV0, v1, v2, pV3, t).GetLength(); }
   ENGINE_API CVector3 const CatmullRomSpline3Acceleration(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t);
   ENGINE_API CVector3 const CatmullRomSpline3Acceleration(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t);
   ENGINE_API CVector3 const CatmullRomSpline3MakeSmoothStart(CVector3 const & v1, CVector3 const & v2, CVector3 const & v3);
   ENGINE_API CVector3 const CatmullRomSpline3MakeSmoothEnd(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2);

   // 4d catmull rom spline
   ENGINE_API CVector4  CatmullRomSpline4(CVector4 const & v0, CVector4 const & v1, CVector4 const & v2, CVector4 const & v3, real32 t);

   // Gets the arc length of a 3d CR spline by approximation
   ENGINE_API real32 CatmullRomSpline3_ArcLength(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t0, real32 const t1);
   // As above, NULL vectors use linear interpolations.
   ENGINE_API real32 CatmullRomSpline3_ArcLength(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t0, real32 const t1);
   
   // Gets the length of a 3d CR spline by approximation
   ENGINE_API real32  CatmullRomSpline3_Length(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, int const numSteps = 10);
   // As above, NULL vectors use linear interpolations.
   ENGINE_API real32  CatmullRomSpline3_Length(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, int const numSteps = 10);

   // Gets the parametric delta of a 3d CR spline by approximation
   ENGINE_API real32  CatmullRomSpline3_ParametricDelta(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const distance, real32 const startT, real32 &remainingDistance, int const numSteps = 10);
   // As above, NULL vectors use linear interpolations.
   ENGINE_API real32  CatmullRomSpline3_ParametricDelta(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const distance, real32 const startT, real32 &remainingDistance, int const numSteps = 10);

   enum ESquad_CatmullRomInterpolateFlags
   {
      // Bitwise flags used for interpolation
      kSCF_None                           = 0,
      kSCF_BuildEquivalentMatrix0         = 1,     // Flips rotation path for m0 quaternion so rotation doesn't change direction on waypoint paths
      kSCF_Terminator
   };

   // Catmull Rom translation and squad rotation interpolate.
   ENGINE_API CMatrix34 Squad_CatmullRomInterpolate( CMatrix34 const &m0, CMatrix34 const &m1, CMatrix34 const &m2, CMatrix34 const &m3, real32 const t, uint32 const flags /* = kNone */ );
   // As above, NULL matrices use linear interpolations.
   ENGINE_API CMatrix34 Squad_CatmullRomInterpolate( CMatrix34 const * const pA, CMatrix34 const &b, CMatrix34 const &c, CMatrix34 const * const pD, real32 const t, uint32 const flags /* = kNone */  );

   // Slerp interpolate between rotation with max angle delta
   ENGINE_API void InterpolateRotation(CMatrix34 const &desiredRotation, real32 const maxAngleDeltaDeg, CMatrix34 &currentRotation_out);
   
   // DriveCubic (see SmoothDriver.h), move to target position and velocity using cubic curve with max accel. 
   // Max_accel should be somewhere around (typical_distance/coverge_time^2).
   // From http://www.cbloom.com/src/SmoothDriver.html   
   ENGINE_API void DriveCubic3d( CVector3 &currentPos, CVector3 &currentVel, 
                                 CVector3 const &toPos, CVector3 const &toVel,
                                 real32 const maxAccel, 
                                 real32 const timeStep);
   
   ENGINE_API void DriveCubic1d( real32 &currentPos, real32 &currentVel, 
                                 real32 const toPos, real32 const toVel,
                                 real32 const maxAccel, 
                                 real32 const timeStep);
   
   // DrivePDClamped (see SmoothDriver.h), move to target position and velocity using PD controller. 
   //	PD damping should generally be between 1.0 and 1.1
   //	frequency should be like 1.0/coverge_time
   // From http://www.cbloom.com/src/SmoothDriver.html
   ENGINE_API void DrivePDClamped3d(CVector3 &currentPos, CVector3 &currentVel, 
                                    CVector3 const &toPos, CVector3 const &toVel, 
                                    real32 const frequency, 
                                    real32 const damping, 
                                    real32 const minVel, 
                                    real32 const timeStep);
                                    
   ENGINE_API void DrivePDClamped1d(real32 &currentPos, real32 &currentVel, 
                                    real32 const toPos, real32 const toVel,
                                    real32 const frequency, 
                                    real32 const damping, 
                                    real32 const minVel, 
                                    real32 const timeStep);

   ENGINE_API bool AngleBetween(CVector2 const &srcUnnormalized, CVector2 const &dstUnnormalized, CAngle &angle_Out);

   inline bool	IsPowerOfTwo(unsigned n)				{ return ((n&(n-1))== 0); }

}

//------------------------------------------------------------------------------------------
