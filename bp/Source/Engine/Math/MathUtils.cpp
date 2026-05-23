//----------------------------------------------------------------------------
// MathUtils.cpp
//
// Catch-all for misc math functions.
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Math/MathUtils.h"
//----------------------------------------------------------------------------

// number of points for Gauss-Legendre intergration
// (determines precision of arc length approximation)
#define GAUSS_LEGENDRE_POINTS 3

//----------------------------------------------------------------------------

namespace
{
   template <typename T> inline T const hermite_spline(T const & p0, T const & m0, T const & p1, T const & m1, real32 const t)
   {
      real32 const t2 = t * t;
      real32 const t3 = t2 * t;
      return
         p0 * ( 2 * t3 - 3 * t2 +     1) +
         m0 * (     t3 - 2 * t2 + t    ) +
         p1 * (-2 * t3 + 3 * t2        ) +
         m1 * (     t3 -     t2        );
   }

   template <typename T> inline T const hermite_spline_velocity(T const & p0, T const & m0, T const & p1, T const & m1, real32 const t)
   {
      real32 const t2 = t * t;
      return
         p0 * ( 6 * t2 - 6 * t    ) +
         m0 * ( 3 * t2 - 4 * t + 1) +
         p1 * (-6 * t2 + 6 * t    ) +
         m1 * ( 3 * t2 - 2 * t    );
   }

   template <typename T> inline T const hermite_spline_acceleration(T const & p0, T const & m0, T const & p1, T const & m1, real32 const t)
   {
      return
         p0 * ( 12 * t - 6) +
         m0 * (  6 * t - 4) +
         p1 * (-12 * t + 6) +
         m1 * (  6 * t - 2);
   }
}

//----------------------------------------------------------------------------

// 1D Hermite spline
real32    MathUtils::HermiteSpline1(real32 const p0, real32 const m0, real32 const p1, real32 const m1, real32 const t)
{
   return hermite_spline<real32>(p0, m0, p1, m1, t);
}

//----------------------------------------------------------------------------

// 3D Hermite spline (position)
CVector3 const MathUtils::HermiteSpline3(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t)
{
   return hermite_spline<CVector3>(p0, m0, p1, m1, t);
}

//----------------------------------------------------------------------------

// 3D Hermite spline (velocity)
CVector3 const MathUtils::HermiteSpline3Velocity(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t)
{
   return hermite_spline_velocity<CVector3>(p0, m0, p1, m1, t);
}

//----------------------------------------------------------------------------

// 3D Hermite spline (acceleration)
CVector3 const MathUtils::HermiteSpline3Acceleration(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t)
{
   return hermite_spline_acceleration<CVector3>(p0, m0, p1, m1, t);
}

//----------------------------------------------------------------------------

real32 MathUtils::HermiteSpline3_Length(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, int const numSteps /*= 20 */)
{
   // integrate using trapezoid rule
   real32 lastVal(HermiteSpline3Speed(p0, m0, p1, m1, 0.0f));   // t = 0
   real32 length = 0.0f;
   for (int loop = 1; loop <= numSteps; loop++)
   {
      real32 const t = ((real32) loop) / numSteps;
      real32 const newVal(HermiteSpline3Speed(p0, m0, p1, m1, t));
      real32 const distance = 0.5f * (newVal + lastVal) / numSteps;
      lastVal = newVal;
      length += distance;
   }

   return length;
}

//----------------------------------------------------------------------------

real32 MathUtils::HermiteSpline3_ArcLength(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const t0, real32 const t1)
{
   // Gauss-Legendre integration over [t0..t1]
   
   // change of variable
   real32 const gauss_scale(0.5f*(t1-t0));
   real32 const gauss_base(0.5f*(t1+t0));

#if GAUSS_LEGENDRE_POINTS==1
   // integrate arc length, N=1
   real32 const arclength = gauss_scale * 
      2.0000000000f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base);
#elif GAUSS_LEGENDRE_POINTS==2
   // integrate arc length, N=2
   real32 const arclength = gauss_scale * (
      HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.5773502692f) +
      HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.5773502692f)
      );
#elif GAUSS_LEGENDRE_POINTS==3
   // integrate arc length, N=3
   real32 const arclength = gauss_scale * (
      0.5555555556f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.7745966692f) +
      0.5555555556f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.7745966692f) +
      0.8888888888f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.0000000000f)
      );
#elif GAUSS_LEGENDRE_POINTS==4   
   // integrate arc length, N=4
   real32 const arclength = gauss_scale * (
      0.3478548451f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.8611363116f) +
      0.3478548451f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.8611363116f) +
      0.6521451549f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.3399810436f) +
      0.6521451549f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.3399810436f)
      );
#elif GAUSS_LEGENDRE_POINTS==5
   // integrate arc length, N=5
   real32 const arclength = gauss_scale * (
      0.2369268851f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.9061798459f) +
      0.2369268851f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.9061798459f) +
      0.4786286705f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.5384693101f) +
      0.4786286705f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.5384693101f) +
      0.5688888888f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.0000000000f)
      );
#elif GAUSS_LEGENDRE_POINTS==6
   // integrate arc length, N=6
   real32 const arclength = gauss_scale * (
      0.1713244924f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.9324695142f) +
      0.1713244924f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.9324695142f) +
      0.3607615730f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.6612093865f) +
      0.3607615730f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.6612093865f) +
      0.4679139346f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.2386191861f) +
      0.4679139346f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.2386191861f)
      );
#elif GAUSS_LEGENDRE_POINTS==7
   // integrate arc length, N=7
   real32 const arclength = gauss_scale * (
      0.1294849662f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.9491079123f) +
      0.1294849662f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.9491079123f) +
      0.2797053915f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.7415311856f) +
      0.2797053915f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.7415311856f) +
      0.3818300505f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.4058451514f) +
      0.3818300505f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.4058451514f) +
      0.4179591837f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.0000000000f)
      );
#elif GAUSS_LEGENDRE_POINTS==8
   // integrate arc length, N=6
   real32 const arclength = gauss_scale * (
      0.1012285363f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.9602898565f) +
      0.1012285363f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.9602898565f) +
      0.2223810345f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.7966664774f) +
      0.2223810345f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.7966664774f) +
      0.3137066459f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.5255324099f) +
      0.3137066459f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.5255324099f) +
      0.3626837834f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale * -0.1834346425f) +
      0.3626837834f * HermiteSpline3Speed(p0, m0, p1, m1, gauss_base + gauss_scale *  0.1834346425f)
      );
#else
#error "GAUSS_LEGENDRE_POINTS must be between 1 and 8"
#endif

   return arclength;
}

/*
// parameterize spline
private function __parameterize():void
{
   // this is a bit innefficient, but will be made tighter in the future.  Place a spline knot at
   // each of the C-R knots and two knots in between.  If spline knots are already in place, then
   // this method was most likely called as a result of moving one or more C-R knots, so regenerate
   // the entire set of interpolation knots.
   if( __param == Consts.ARC_LENGTH )
   {
      if( __arcLength == -1 )
         var len:Number = arcLength();

      var normalize:Number = 1.0/__arcLength;

      if( __spline.knotCount > 0 )
         __spline.deleteAllKnots();

      // x-coordinate of spline knot is normalized arc-length, y-coordinate is t-value for uniform parameterization
      __spline.addControlPoint(0.0, 0.0);
      var prevT:Number    = 0;
      var knotsInv:Number = 1.0/Number(__knots-1);

      for( var i:uint=1; i<__knots-1; i++ )
      {
         // get t-value at this knot for uniform parameterization
         var t:Number  = Number(i)*knotsInv;
         var t1:Number = prevT + Consts.ONE_THIRD*(t-prevT);
         var l:Number  = arcLengthAt(t1)*normalize;
         __spline.addControlPoint(l,t1);

         var t2:Number = prevT + Consts.TWO_THIRDS*(t-prevT);
         l             = arcLengthAt(t2)*normalize;
         __spline.addControlPoint(l,t2);

         l = arcLengthAt(t)*normalize;
         __spline.addControlPoint(l,t);

         prevT = t;
      }

      t1 = prevT + Consts.ONE_THIRD*(1.0-prevT);
      l  = arcLengthAt(t1)*normalize;
      __spline.addControlPoint(l,t1);

      t2 = prevT + Consts.TWO_THIRDS*(1.0-prevT);
      l  = arcLengthAt(t2)*normalize;
      __spline.addControlPoint(l,t2);

      // last knot, t=1, normalized arc-length = 1
      __spline.addControlPoint(1.0, 1.0);
   }
}
*/

/*
// compute z[i] based on current knots
private function __computeZ():void
{
   // reference the white paper for details on this code

   // pre-generate h^-1 since the same quantity could be repeatedly calculated in eval()
   for( var i:uint=0; i<__knots-1; ++i )
   {
      __h[i]    = __t[i+1] - __t[i];
      __hInv[i] = 1.0/__h[i];
      __b[i]    = (__y[i+1] - __y[i])*__hInv[i];
   }

   // recurrence relations for u(i) and v(i) -- tridiagonal solver
   __u[1] = 2.0*(__h[0]+__h[1]);
   __v[1] = 6.0*(__b[1]-__b[0]);

   for( i=2; i<__knots-1; ++i )
   {
      __u[i] = 2.0*(__h[i]+__h[i-1]) - (__h[i-1]*__h[i-1])/__u[i-1];
      __v[i] = 6.0*(__b[i]-__b[i-1]) - (__h[i-1]*__v[i-1])/__u[i-1];
   }

   // compute z(i)
   __z[__knots-1] = 0.0;
   for( i=__knots-2; i>=1; i-- )
      __z[i] = (__v[i]-__h[i]*__z[i+1])/__u[i];

   __z[0] = 0.0;

   __invalidate = false;
}
*/

/*
public function eval(_xKnot:Number):Number
{
   if( __knots == 0 )
      return NaN;
   else if( __knots == 1 )
      return __y[0];
   else
   {
      if( __invalidate )
         __computeZ();

      // determine interval
      var i:uint = 0;
      __delta    = _xKnot - __t[0];
      for( var j:uint=__knots-2; j>=0; j-- )
      {
         if( _xKnot >= __t[j] )
         {
            __delta = _xKnot - __t[j];
            i = j;
            break;
         }
      }

      var b:Number = (__y[i+1] - __y[i])*__hInv[i] - __h[i]*(__z[i+1] + 2.0*__z[i])*0.1666666666666667;
      var q:Number = 0.5*__z[i] + __delta*(__z[i+1]-__z[i])*0.1666666666666667*__hInv[i];
      var r:Number = b + __delta*q;
      var s:Number = __y[i] + __delta*r;

      return s;
   }
}
*/

/*
static real32 HermiteSpline3_Parameterize(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 distance)
{
   // knots = 4

   // parameter -> y
   real32 const y[4] = 
   {
      0.0f,
      1.0f/3.0f,
      2.0f/2.0f,
      1.0f
   };
   
   // arc length -> x
   real32 const x[4] =
   {
      0.0f,
      MathUtils::HermiteSpline3_ArcLength(p0, m0, p1, m1, 0.0f, y[1]),
      MathUtils::HermiteSpline3_ArcLength(p0, m0, p1, m1, 0.0f, y[2]),
      MathUtils::HermiteSpline3_ArcLength(p0, m0, p1, m1, 0.0f, y[3]),
   };
   
   real32 z[4];
   real32 h[3], hInv[3];
   
   {
      // pre-generate h^-1 since the same quantity could be repeatedly calculated in eval()
      real32 b[3];
      for (int i = 0; i < 3; ++i)
      {
         h[i] = x[i+1] - x[i];
         hInv[i] = 1.0f / h[i];
         b[i] = (y[i+1] - y[i]) * hInv[i];
      }

      // recurrence relations for u(i) and v(i) -- tridiagonal solver
      real32 u[3], v[3];
      u[0] = 2.0f * (h[1] + h[0]);
      v[0] = 6.0f * (b[1] - b[0]);
      for (int i = 1; i < 3; ++i)
      {
         u[i] = 2.0f * (h[i+1] + h[i]) - (h[i] * h[i]) / u[i-1];
         v[i] = 6.0f * (b[i+1] - b[i]) - (h[i] * v[i]) / u[i-1];
      }
      
      // compute z(i)
      z[3] = 0.0f;
      for (int i = 2; i >= 1; --i)
         z[i] = (v[i] - h[i] * z[i+1]) / u[i-1];
      z[0] = 0.0f;
   }
   
   // determine interval
   int i = 0;
   real32 delta = distance - x[0];
   for (int j = 2; j >= 0; --j)
   {
      if( distance >= x[j] )
      {
         delta = distance - x[j];
         i = j;
         break;
      }
   }

   // evaluate
   real32 const b = (y[i+1] - y[i]) * hInv[i] - h[i] * (z[i+1] + 2.0f * z[i]) * 0.1666666666666667f;
   real32 const q = 0.5f * z[i] + delta * (z[i+1] - z[i]) * 0.1666666666666667f * hInv[i];
   real32 const r = b + delta * q;
   real32 const s = y[i] + delta * r;

   return s;
}
*/

//----------------------------------------------------------------------------
real32 MathUtils::HermiteSpline3_ParametricDelta(CVector3 const & p0, CVector3 const & m0, CVector3 const & p1, CVector3 const & m1, real32 const distance, real32 const startT, real32 &remainingDistance, int const numSteps /* = 20 */)
{
   if (distance < gkEpsilon32)
   {
      remainingDistance = distance;
      return startT;
   }

#if 1

   // remaining distance
   real32 remaining = distance;

   // current estimate of parameter
   real32 t_curr(startT);

   // get speed at the current estimate
   real32 v_curr(MathUtils::HermiteSpline3Speed(p0, m0, p1, m1, t_curr));

   // iteration loop
   for (int iter = 0; iter < numSteps; ++iter)
   {
      // next estimate of parameter
      real32 const t_next(MathUtils::ClampMinMax(t_curr + remaining / v_curr, 0.0f, 1.0f));

      // stop if close enough
      if (fabsf(t_next - t_curr) < gkEpsilon32)
         break;

      // get speed at the next estimate
      real32 const v_next(MathUtils::HermiteSpline3Speed(p0, m0, p1, m1, t_curr));

      // integrate using trapezoid rule
      remaining -= 0.5f * (t_next - t_curr) * (v_next + v_curr);

      // update current values
      t_curr = t_next;
      v_curr = v_next;
   }

   // if stopping within this segment...
   if (t_curr < 1.0f)
   {
      // zero out remaining distance
      remaining = 0.0f;
   }

   // return value
   remainingDistance = remaining;
   return t_curr;

#else

   real32 lastVal(HermiteSpline3Speed(p0, m0, p1, m1, startT));
   real32 const deltaT = (1.0f - startT) / numSteps;
   real32 length = 0.0f;
   for (int loop = 1; loop <= numSteps; loop++)
   {
      real32 const stepT = startT + (deltaT * loop);
      real32 const newVal(HermiteSpline3Speed(p0, m0, p1, m1, stepT));
      real32 const stepDistance = 0.5f*deltaT*(newVal+lastVal);
      length += stepDistance;
      if (length >= distance)
      {
         // Passed distance
         if (stepDistance < gkEpsilon32)
         {
            remainingDistance = 0.0f;
            return stepT;
         }
         real32 const stepDistanceDelta = length - distance;
         // Lerp t as approximation
         real32 const t = stepT - ((deltaT * stepDistanceDelta) / stepDistance);
         remainingDistance = 0.0f;
         return t;
      }
      lastVal = newVal;
   }
   // Reached end
   remainingDistance = distance - length;
   return 1.0f;

#endif
}

//----------------------------------------------------------------------------

// Clean this up later!
// Coefficients for Matrix M
#define M11	 0.0f	
#define M12	 1.0f
#define M13	 0.0f
#define M14	 0.0f
#define M21	-0.5f
#define M22	 0.0f
#define M23	 0.5f
#define M24	 0.0f
#define M31	 1.0f
#define M32	-2.5f
#define M33	 2.0f
#define M34	-0.5f
#define M41	-0.5f
#define M42	 1.5f
#define M43	-1.5f
#define M44	 0.5f

//----------------------------------------------------------------------------

namespace
{
   template<class Type> inline Type const catmull_rom_spline(Type const &v0, Type const &v1, Type const &v2, Type const &v3, real32 const x)
   {
      real32 const x2 = x*x;
      real32 const x3 = x*x2;

      return
         v0 * (M41 * x3 + M31 * x2 + M21 * x      ) +
         v1 * (M42 * x3 + M32 * x2           + M12) +
         v2 * (M43 * x3 + M33 * x2 + M23 * x      ) +
         v3 * (M44 * x3 + M34 * x2                );
   }

   template<class Type> inline Type const catmull_rom_spline_velocity(Type const &v0, Type const &v1, Type const &v2, Type const &v3, real32 const x)
   {
      real32 const dx2 = 2*x;
      real32 const dx3 = 3*x*x;

      return
         v0 * (M41 * dx3 + M31 * dx2 + M21) +
         v1 * (M42 * dx3 + M32 * dx2      ) +
         v2 * (M43 * dx3 + M33 * dx2 + M23) +
         v3 * (M44 * dx3 + M34 * dx2      );
   }

   template<class Type> inline Type const catmull_rom_spline_acceleration(Type const &v0, Type const &v1, Type const &v2, Type const &v3, real32 const x)
   {
      real32 const ddx2 = 2;
      real32 const ddx3 = 6*x;

      return
         v0 * (M41 * ddx3 + M31 * ddx2) +
         v1 * (M42 * ddx3 + M32 * ddx2) +
         v2 * (M43 * ddx3 + M33 * ddx2) +
         v3 * (M44 * ddx3 + M34 * ddx2);
   }

   template<class Type> inline Type const catmull_rom_spline_make_smooth_start(Type const &v1, Type const &v2, Type const &v3)
   {
      // zero acceleration at x = 0
      // c3 = 0
      return -(M32*v1 + M33*v2 + M34*v3)/M31;
   }

   template<class Type> inline Type const catmull_rom_spline_make_smooth_end(Type const &v0, Type const &v1, Type const &v2)
   {
      // zero acceleration at x = 1
      // 3*c4 + c3 = 0
      return -((3.0f*M41 + M31)*v0 + (3.0f*M42 + M32)*v1 + (3.0f*M43 + M33)*v2)/(3.0f*M44 + M34);
   }
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline1(real32 v0, real32 v1, real32 v2, real32 v3, real32 x)
{
   return catmull_rom_spline<real32>(v0, v1, v2, v3, x);
}

//----------------------------------------------------------------------------

void GetEndpoints(CVector3 &v0, CVector3 &v3, CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3)
{
   if (pV0 && pV3)
   {
      v0 = *pV0;
      v3 = *pV3;
      return;
   }

   CVector3 const translationDelta(v2 - v1);
   CVector3 normalizedTranslationDelta(translationDelta);
   normalizedTranslationDelta.TryNormalize_WSDelta();

   if (!pV0)
   {
      v0 = v1;
      v0 -= normalizedTranslationDelta;
   }
   else
   {
      v0 = *pV0;
   }
   if (!pV3)
   {
      v3 = v2;
      v3 -= normalizedTranslationDelta;
   }
   else
   {
      v3 = *pV3;
   }
}

//----------------------------------------------------------------------------

CVector3 const MathUtils::CatmullRomSpline3(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t)
{
   return catmull_rom_spline<CVector3>(v0, v1, v2, v3, t);
}

CVector3 const MathUtils::CatmullRomSpline3(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 t)
{
   CVector3 v0(CVector3::kConstructUninitialized);
   CVector3 v3(CVector3::kConstructUninitialized);
   GetEndpoints(v0, v3, pV0, v1, v2, pV3);
   return catmull_rom_spline<CVector3>(v0, v1, v2, v3, t);
}

//----------------------------------------------------------------------------

CVector3 const MathUtils::CatmullRomSpline3Velocity(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 t)
{
   return catmull_rom_spline_velocity<CVector3>(v0, v1, v2, v3, t);
}

CVector3 const MathUtils::CatmullRomSpline3Velocity(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t)
{
   CVector3 v0(CVector3::kConstructUninitialized);
   CVector3 v3(CVector3::kConstructUninitialized);
   GetEndpoints(v0, v3, pV0, v1, v2, pV3);
   return catmull_rom_spline_velocity<CVector3>(v0, v1, v2, v3, t);
}

//----------------------------------------------------------------------------

CVector3 const MathUtils::CatmullRomSpline3Acceleration(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 t)
{
   return catmull_rom_spline_acceleration<CVector3>(v0, v1, v2, v3, t);
}

CVector3 const MathUtils::CatmullRomSpline3Acceleration(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 t)
{
   CVector3 v0(CVector3::kConstructUninitialized);
   CVector3 v3(CVector3::kConstructUninitialized);
   GetEndpoints(v0, v3, pV0, v1, v2, pV3);
   return catmull_rom_spline_acceleration<CVector3>(v0, v1, v2, v3, t);
}

//----------------------------------------------------------------------------

CVector3 const MathUtils::CatmullRomSpline3MakeSmoothStart(CVector3 const & v1, CVector3 const & v2, CVector3 const & v3)
{
   return catmull_rom_spline_make_smooth_start<CVector3>(v1, v2, v3);
}

//----------------------------------------------------------------------------

CVector3 const MathUtils::CatmullRomSpline3MakeSmoothEnd(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2)
{
   return catmull_rom_spline_make_smooth_end<CVector3>(v0, v1, v2);
}

//----------------------------------------------------------------------------

CVector4 MathUtils::CatmullRomSpline4(CVector4 const & v0, CVector4 const & v1, CVector4 const & v2, CVector4 const & v3, real32 t)
{
   return catmull_rom_spline<CVector4>(v0, v1, v2, v3, t);
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline3_Length(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, int const numSteps /*= 20 */)
{
   // integrate using trapezoid rule
   real32 lastVal(CatmullRomSpline3Speed(v0, v1, v2, v3, 0.0f));   // t = 0
   real32 length = 0.0f;
   for (int loop = 1; loop <= numSteps; loop++)
   {
      real32 const t = ((real32) loop) / numSteps;
      real32 const newVal(CatmullRomSpline3Speed(v0, v1, v2, v3, t));
      real32 const distance = 0.5f * (lastVal + newVal) / numSteps;
      lastVal = newVal;
      length += distance;
   }

   return length;
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline3_Length(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, int const numSteps /*= 20 */)
{
   CVector3 v0(CVector3::kConstructUninitialized);
   CVector3 v3(CVector3::kConstructUninitialized);
   GetEndpoints(v0, v3, pV0, v1, v2, pV3);
   return CatmullRomSpline3_Length(v0, v1, v2, v3, numSteps);
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline3_ArcLength(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const t0, real32 const t1)
{
   // Gauss-Legendre integration over [t0..t1]

   // change of variable
   real32 const gauss_scale(0.5f*(t1-t0));
   real32 const gauss_base(0.5f*(t1+t0));

#if GAUSS_LEGENDRE_POINTS==1
   // integrate arc length, N=1
   real32 const arclength = gauss_scale * 
      2.0000000000f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base);
#elif GAUSS_LEGENDRE_POINTS==2
   // integrate arc length, N=2
   real32 const arclength = gauss_scale * (
      CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.5773502692f) +
      CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.5773502692f)
      );
#elif GAUSS_LEGENDRE_POINTS==3
   // integrate arc length, N=3
   real32 const arclength = gauss_scale * (
      0.5555555556f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.7745966692f) +
      0.5555555556f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.7745966692f) +
      0.8888888888f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.0000000000f)
      );
#elif GAUSS_LEGENDRE_POINTS==4   
   // integrate arc length, N=4
   real32 const arclength = gauss_scale * (
      0.3478548451f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.8611363116f) +
      0.3478548451f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.8611363116f) +
      0.6521451549f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.3399810436f) +
      0.6521451549f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.3399810436f)
      );
#elif GAUSS_LEGENDRE_POINTS==5
   // integrate arc length, N=5
   real32 const arclength = gauss_scale * (
      0.2369268851f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.9061798459f) +
      0.2369268851f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.9061798459f) +
      0.4786286705f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.5384693101f) +
      0.4786286705f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.5384693101f) +
      0.5688888888f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.0000000000f)
      );
#elif GAUSS_LEGENDRE_POINTS==6
   // integrate arc length, N=6
   real32 const arclength = gauss_scale * (
      0.1713244924f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.9324695142f) +
      0.1713244924f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.9324695142f) +
      0.3607615730f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.6612093865f) +
      0.3607615730f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.6612093865f) +
      0.4679139346f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.2386191861f) +
      0.4679139346f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.2386191861f)
      );
#elif GAUSS_LEGENDRE_POINTS==7
   // integrate arc length, N=7
   real32 const arclength = gauss_scale * (
      0.1294849662f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.9491079123f) +
      0.1294849662f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.9491079123f) +
      0.2797053915f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.7415311856f) +
      0.2797053915f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.7415311856f) +
      0.3818300505f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.4058451514f) +
      0.3818300505f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.4058451514f) +
      0.4179591837f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.0000000000f)
      );
#elif GAUSS_LEGENDRE_POINTS==8
   // integrate arc length, N=6
   real32 const arclength = gauss_scale * (
      0.1012285363f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.9602898565f) +
      0.1012285363f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.9602898565f) +
      0.2223810345f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.7966664774f) +
      0.2223810345f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.7966664774f) +
      0.3137066459f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.5255324099f) +
      0.3137066459f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.5255324099f) +
      0.3626837834f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale * -0.1834346425f) +
      0.3626837834f * CatmullRomSpline3Speed(v0, v1, v2, v3, gauss_base + gauss_scale *  0.1834346425f)
      );
#else
#error "GAUSS_LEGENDRE_POINTS must be between 1 and 8"
#endif

   return arclength;
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline3_ArcLength(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const t0, real32 const t1)
{
   CVector3 v0(CVector3::kConstructUninitialized);
   CVector3 v3(CVector3::kConstructUninitialized);
   GetEndpoints(v0, v3, pV0, v1, v2, pV3);
   return CatmullRomSpline3_ArcLength(v0, v1, v2, v3, t0, t1);
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline3_ParametricDelta(CVector3 const & v0, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, real32 const distance, real32 const startT, real32 &remainingDistance, int const numSteps /* = 20 */)
{
   if (distance < gkEpsilon32)
   {
      remainingDistance = distance;
      return startT;
   }

#if 1

   // remaining distance
   real32 remaining = distance;

   // current estimate of parameter
   real32 t_curr(startT);

   // get speed at the current estimate
   real32 v_curr(MathUtils::CatmullRomSpline3Speed(v0, v1, v2, v3, t_curr));

   // iteration loop
   for (int iter = 0; iter < numSteps; ++iter)
   {
      // next estimate of parameter
      real32 const t_next(MathUtils::ClampMinMax(t_curr + remaining / v_curr, 0.0f, 1.0f));

      // stop if close enough
      if (fabsf(t_next - t_curr) < gkEpsilon32)
         break;

      // get speed at the next estimate
      real32 const v_next(MathUtils::CatmullRomSpline3Speed(v0, v1, v2, v3, t_curr));

      // integrate using trapezoid rule
      remaining -= 0.5f * (t_next - t_curr) * (v_next + v_curr);

      // update current values
      t_curr = t_next;
      v_curr = v_next;
   }

   // if stopping within this segment...
   if (t_curr < 1.0f)
   {
      // zero out remaining distance
      remaining = 0.0f;
   }

   // return value
   remainingDistance = remaining;
   return t_curr;

#else

   // integrate using trapezoid rule
   real32 lastVal(CatmullRomSpline3Speed(v0, v1, v2, v3, startT));
   real32 const deltaT = (1.0f - startT) / numSteps;
   real32 length = 0.0f;
   for (int loop = 1; loop <= numSteps; loop++)
   {
      real32 const stepT = startT + (deltaT * loop);
      real32 const newVal(CatmullRomSpline3Speed(v0, v1, v2, v3, stepT));
      real32 const stepDistance = 0.5f*deltaT*(newVal+lastVal);
      length += stepDistance;
      if (length >= distance)
      {
         // Passed distance
         if (stepDistance < gkEpsilon32)
         {
            remainingDistance = 0.0f;
            return stepT;
         }
         real32 const stepDistanceDelta = length - distance;
         // Lerp t as approximation
         real32 const t = stepT - ((deltaT * stepDistanceDelta) / stepDistance);
         remainingDistance = 0.0f;
         return t;
      }
      lastVal = newVal;
   }
   // Reached end
   remainingDistance = distance - length;
   return 1.0f;

#endif
}

//----------------------------------------------------------------------------

real32 MathUtils::CatmullRomSpline3_ParametricDelta(CVector3 const * const pV0, CVector3 const & v1, CVector3 const & v2, CVector3 const * const pV3, real32 const distance, real32 const startT, real32 &remainingDistance, int const numSteps /* = 20 */)
{
   CVector3 v0(CVector3::kConstructUninitialized);
   CVector3 v3(CVector3::kConstructUninitialized);
   GetEndpoints(v0, v3, pV0, v1, v2, pV3);
   return CatmullRomSpline3_ParametricDelta(v0, v1, v2, v3, distance, startT, remainingDistance, numSteps);
}

//----------------------------------------------------------------------------

#if !defined(SPU)
CMatrix34 MathUtils::Squad_CatmullRomInterpolate( CMatrix34 const * const pA, CMatrix34 const &b, CMatrix34 const &c, CMatrix34 const * const pD, real32 const t, uint32 const flags)
{
   if (pA && pD)
   {
      return Squad_CatmullRomInterpolate(*pA, b, c, *pD, t, flags);
   }

   CVector3 const translationDelta(c.GetTranslation() - b.GetTranslation());
   CVector3 normalizedTranslationDelta(translationDelta);
   normalizedTranslationDelta.TryNormalize_WSDelta();
   
   CMatrix34 a(CMatrix34::kConstructUninitialized);
   if (!pA)
   {
      a = b;
      a.AddTranslation(-normalizedTranslationDelta);
   }
   else
   {
      a = *pA;
   }

   CMatrix34 d(CMatrix34::kConstructUninitialized);
   if (!pD)
   {
      d = c;
      d.AddTranslation(normalizedTranslationDelta);
   }
   else
   {
      d = *pD;
   }

   return Squad_CatmullRomInterpolate(a, b, c, d, t, flags);
}


//----------------------------------------------------------------------------
#include "CQuaternion.h"

CMatrix34 MathUtils::Squad_CatmullRomInterpolate( CMatrix34 const &m0, CMatrix34 const &m1, CMatrix34 const &m2, CMatrix34 const &m3, real32 const t, uint32 const flags )
{
   CQuaternion q0(CQuaternion::FromMatrix34(m0));
   CQuaternion q1(CQuaternion::FromMatrix34(m1));
   CQuaternion q2(CQuaternion::FromMatrix34(m2));
   CQuaternion q3(CQuaternion::FromMatrix34(m3));

   if ((flags & kSCF_BuildEquivalentMatrix0) != 0)
   {
      // Change rotation interpolate direction for first matrix
      q0 = CQuaternion::BuildEquivalent(q0);
   }
   // Make all rotations local (shortest path) to each other
   q1.MakeLocalTo(q0);
   q2.MakeLocalTo(q1);
   q3.MakeLocalTo(q2);

   CQuaternion const rotation(CQuaternion::Spline(q0, q1, q2, q3, t));
   CVector3 const translation(CatmullRomSpline3(m0.GetTranslation(), m1.GetTranslation(), m2.GetTranslation(), m3.GetTranslation(), t));

   CMatrix34 const transform(CMatrix34::FromQuaternion(rotation, translation));
   return transform;
}

//----------------------------------------------------------------------------

void MathUtils::InterpolateRotation(CMatrix34 const &desiredRotation, real32 const maxAngleDeltaDeg, CMatrix34 &currentRotation_out)
{
   CQuaternion const desiredQ(CQuaternion::FromMatrix34(desiredRotation));
   CQuaternion currentQ(CQuaternion::FromMatrix34(currentRotation_out));
   currentQ.MakeLocalTo(desiredQ);
   real32 const angle = CQuaternion::Angle(desiredQ, currentQ);
   real32 const maxAngleDelta = maxAngleDeltaDeg * gkDegrees2Radians32;

   CQuaternion newQ(desiredQ);   // Assume full re-orient
   if (angle > maxAngleDelta)
   {
      // Nope, too far, need to slerp.
      real32 const slerpT = maxAngleDelta / angle;
      newQ = CQuaternion::SlerpAccurate(currentQ, desiredQ, slerpT);
   }
   currentRotation_out = CMatrix34::FromQuaternion(newQ);
}

//----------------------------------------------------------------------------

#include "Engine/Math/SmoothDriver.h"

void MathUtils::DriveCubic3d(CVector3 &currentPos, CVector3 &currentVel, 
                              CVector3 const &toPos, CVector3 const &toVel,
                              real32 const maxAccel, 
                              real32 const timeStep)
{
   SmoothDriver::DriveCubic((SmoothDriver::Vec3*)&currentPos, (SmoothDriver::Vec3*)&currentVel, 
                            (SmoothDriver::Vec3 const *)&toPos, (SmoothDriver::Vec3 const *)&toVel, 
                            maxAccel, timeStep);                              
}
   
//----------------------------------------------------------------------------

void MathUtils::DrivePDClamped3d(CVector3 &currentPos, CVector3 &currentVel, 
                                  CVector3 const &toPos, CVector3 const &toVel, 
                                  real32 const frequency, 
                                  real32 const damping, 
                                  real32 const minVel, 
                                  real32 const timeStep)
{
   SmoothDriver::DrivePDClamped( (SmoothDriver::Vec3*)&currentPos, (SmoothDriver::Vec3*)&currentVel, 
                                 (SmoothDriver::Vec3 const *)&toPos, (SmoothDriver::Vec3 const *)&toVel, 
                                 frequency, damping, minVel, timeStep);                              
}                                  

//----------------------------------------------------------------------------
// Placeholder 1d versions
void MathUtils::DriveCubic1d( real32 &currentPos, real32 &currentVel, 
                                 real32 const toPos, real32 const toVel,
                                 real32 const maxAccel, 
                                 real32 const timeStep)
{
   CVector3 _currentPos(currentPos, 0, 0);
   CVector3 _currentVel(currentVel, 0, 0);
   CVector3 _toPos(toPos, 0, 0);
   CVector3 _toVel(toVel, 0, 0);
   SmoothDriver::DriveCubic((SmoothDriver::Vec3*)&_currentPos, (SmoothDriver::Vec3*)&_currentVel, 
                            (SmoothDriver::Vec3 const *)&_toPos, (SmoothDriver::Vec3 const *)&_toVel, 
                            maxAccel, timeStep);                              
                            
   currentPos = _currentPos.mX;                            
   currentVel = _currentVel.mX;
}

//----------------------------------------------------------------------------

void MathUtils::DrivePDClamped1d(real32 &currentPos, real32 &currentVel, 
                                 real32 const toPos, real32 const toVel,
                                 real32 const frequency, 
                                 real32 const damping, 
                                 real32 const minVel, 
                                 real32 const timeStep)
{
   CVector3 _currentPos(currentPos, 0, 0);
   CVector3 _currentVel(currentVel, 0, 0);
   CVector3 _toPos(toPos, 0, 0);
   CVector3 _toVel(toVel, 0, 0);
   SmoothDriver::DrivePDClamped( (SmoothDriver::Vec3*)&_currentPos, (SmoothDriver::Vec3*)&_currentVel, 
                                 (SmoothDriver::Vec3 const *)&_toPos, (SmoothDriver::Vec3 const *)&_toVel, 
                                 frequency, damping, minVel, timeStep);                              
   currentPos = _currentPos.mX;                            
   currentVel = _currentVel.mX;
}                                  

//----------------------------------------------------------------------------

bool MathUtils::AngleBetween(CVector2 const &srcUnnormalized, CVector2 const &dstUnnormalized, CAngle &angle_Out)
{
   CVector2 src = srcUnnormalized;
   CVector2 dst = dstUnnormalized;
   if (src.TryNormalize() && dst.TryNormalize())
   {
      angle_Out = CAngle::FromRadians(atan2f(CVector2::Cross(src, dst), CVector2::Dot(src, dst)));
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------

#endif

