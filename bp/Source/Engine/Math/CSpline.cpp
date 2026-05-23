//----------------------------------------------------------------------------
// CSpline.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSpline.h"

//----------------------------------------------------------------------------

#include "Engine/Math/CloseEnough.h"
#include "Engine/Streams/CLinearCopyStream.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

real32 const kMaxTan = 5729577.9485111479f;
real32 const kFourThirds = (4.0f / 3.0f);
real32 const kTwoThirds = (2.0f / 3.0f);
real32 const kOneThird = (1.0f / 3.0f);

//----------------------------------------------------------------------------
// Removed unneeded output code when compiling for SPU's
#if !defined(SPU)

//----------------------------------------------------------------------------

CSpline::CSpline(CInputStream& stream)
:  mPreInfinity( static_cast<EInfinity>(stream.ReadInt8()) )
,  mPostInfinity( static_cast<EInfinity>(stream.ReadInt8()) )
,  mIsWeighted( stream.ReadBool() )
,  mKeysSorted( true )
,  mKeys( stream )
{
}

//----------------------------------------------------------------------------

void CSpline::ConstructFromStream(CInputStream & stream)
{
   mPreInfinity = static_cast<EInfinity>(stream.ReadInt8());
   mPostInfinity = static_cast<EInfinity>(stream.ReadInt8());
   mIsWeighted =  stream.ReadBool();
   mKeysSorted = true;
   mKeys.ConstructFromStream( stream );
}

//----------------------------------------------------------------------------

void CSpline::PutTo(COutputStream & stream) const
{
   stream.WriteInt8(mPreInfinity);
   stream.WriteInt8(mPostInfinity);
   stream.WriteBool(mIsWeighted);
   EnsureKeysSorted();
   stream.Put(mKeys);
}

//----------------------------------------------------------------------------

void CSpline::CopyToLinearStream(CLinearCopyStream &stream) const
{
   // Copy members.
   int bufferPos = stream.Put(this, sizeof(*this));
   // Copy vector and fixup pointers.
   stream.PutPODVector<CSplineKey>(mKeys, bufferPos + (int) ((uint8*) &mKeys - (uint8*) this));
}

//----------------------------------------------------------------------------

void CSpline::CopyToLinearStream_Inplace(CLinearCopyStream &stream, int bufferPos) const
{
   // Members are already in buffer at bufferPos.
   // Copy vector and fixup pointers.
   stream.PutPODVector<CSplineKey>(mKeys, bufferPos + (int) ((uint8*) &mKeys - (uint8*) this));
}

//----------------------------------------------------------------------------

CSpline::~CSpline()
{
}

//----------------------------------------------------------------------------

CSpline::CSpline(bool const isWeighted,
                 EInfinity const preInfinity,
                 EInfinity const postInfinity)
:  mKeysSorted(true)
,  mIsWeighted(isWeighted)
,  mPreInfinity(preInfinity)
,  mPostInfinity(postInfinity)
{
}

//----------------------------------------------------------------------------

void CSpline::AddKey( CSplineKey const & key )
{
   if( !mKeys.empty() && (key.mTime < mKeys.back().mTime) )
   {
      mKeysSorted = false;
   }
   
   mKeys.push_back( key );
}

//----------------------------------------------------------------------------

void CSpline::EnsureKeysSorted() const
{
   if( !mKeysSorted )
   {
      CSpline * pThis = const_cast<CSpline*>(this);
      pThis->mKeysSorted = true;
      std::sort( pThis->mKeys.begin(), pThis->mKeys.end() );
   }
}
#else //!defined(SPU)

// No key sorting on SPU
inline void CSpline::EnsureKeysSorted() const
{
}

#endif //!defined(SPU)
//----------------------------------------------------------------------------

real32 const CSpline::EvaluateInfinities(real32 const time, bool evalPre, SSplineEvaluationCache * const pCache) const
{
   float value = 0.0;

   // make sure we have something to evaluate
   if( mKeys.empty() )
   {
      return value;
   }

   // find the number of cycles of the base animation curve
   real32 firstTime = mKeys.front().mTime;
   real32 lastTime = mKeys[mKeys.size() - 1].mTime;
   real32 timeRange = lastTime - firstTime;
   if( close_enough( timeRange, 0.0f ) )
   {
      // Means that there is only one key in the curve.. Return the value
      // of that key..
      return mKeys.front().mValue;
   }

   real32 remainder, tanX, tanY;
   double numCycles, notUsed;

   if( time > lastTime )
   {
      remainder = (real32)fabs( modf((time - lastTime) / timeRange, &numCycles));
   }
   else
   {
      remainder = (real32)fabs( modf((time - firstTime) / timeRange, &numCycles));
   }

   real32 factoredTime = timeRange * remainder;
   numCycles = fabs(numCycles) + 1;

   if( evalPre )
   {
      // evaluate the pre-infinity
      if( mPreInfinity == kInfinity_Oscillate )
      {
         remainder = (real32)modf(numCycles / 2.0, &notUsed);
         if( remainder != 0.0f )
         {
            factoredTime = firstTime + factoredTime;
         }
         else
         {
            factoredTime = lastTime - factoredTime;
         }
      }
      else if( (mPreInfinity == kInfinity_Cycle) || (mPreInfinity == kInfinity_CycleRelative) )
      {
         factoredTime = lastTime - factoredTime;
      }
      else if( mPreInfinity == kInfinity_Linear )
      {
         factoredTime = firstTime - time;
         tanX = mKeys.front().mInTangent.mX;
         tanY = mKeys.front().mInTangent.mY;
         value = mKeys.front().mValue;

         if( tanX != 0.0 )
         {
            value -= ((factoredTime * tanY) / tanX);
         }
         return(value);
      }
   }
   else
   {
      // evaluate the post-infinity
      if( mPostInfinity == kInfinity_Oscillate )
      {
         remainder = (real32)modf (numCycles / 2.0, &notUsed);
         if( remainder != 0.0 )
         {
            factoredTime = lastTime - factoredTime;
         }
         else
         {
            factoredTime = firstTime + factoredTime;
         }
      }
      else if( (mPostInfinity == kInfinity_Cycle) || (mPostInfinity == kInfinity_CycleRelative) )
      {
         factoredTime = firstTime + factoredTime;
      }
      else if( mPostInfinity == kInfinity_Linear )
      {
         factoredTime = time - lastTime;
         tanX = mKeys[mKeys.size() - 1].mOutTangent.mX;
         tanY = mKeys[mKeys.size() - 1].mOutTangent.mY;
         value = mKeys[mKeys.size() - 1].mValue;
         if( tanX != 0.0 )
         {
            value += ((factoredTime * tanY) / tanX);
         }
         return(value);
      }
   }

   value = Evaluate(factoredTime, pCache);

   float valueRange = 0.0f;

   // Modify the value if infinityType is cycleRelative
   if( evalPre && (mPreInfinity == kInfinity_CycleRelative) )
   {
      valueRange = mKeys[mKeys.size() - 1].mValue - mKeys.front().mValue;
      value -= (real32)(numCycles * valueRange);
   }
   else if( !evalPre && (mPostInfinity == kInfinity_CycleRelative) )
   {
      valueRange = mKeys[mKeys.size() - 1].mValue - mKeys.front().mValue;
      value += (real32)(numCycles * valueRange);
   }
   return(value);
}

//----------------------------------------------------------------------------

//	Function Name:
//		find
//
//	Description:
//		A static helper method to find a key prior to a specified time
//
//  Input Arguments:
//		EtCurve *animCurve			The animation curve to search
//		EtTime time					The time (in seconds) to find
//		EtInt *index				The index of the key prior to time
//
//  Return Value:
//      EtBoolean result
//			kEngineTRUE				time is represented by an actual key
//										(with the index in index)
//			kEngineFALSE			the index key is the key less than time
//
//	Note:
//		keys are sorted by ascending time, which means we can use a binary
//	search to find the key

bool CSpline::FindKey(real32 time, int *index) const
{
   int mid, low, high;

   // use a binary search to find the key
   *index = 0;
   int const len = mKeys.size();
   if( len > 0 )
   {
      low = 0;
      high = len - 1;
      do
      {
         mid = (low + high) >> 1;
         if( time < mKeys[mid].mTime )
         {
            high = mid - 1;         // Search lower half
         }
         else if( time > mKeys[mid].mTime )
         {
            low  = mid + 1;         // Search upper half
         }
         else
         {
            *index = mid;  // Found item!
            return true;
         }
      }
      while( low <= high );
      *index = low;
   }
   return false;
}

//----------------------------------------------------------------------------

real32 const CSpline::Evaluate( real32 const time, SSplineEvaluationCache * const pCache ) const
{
   EnsureKeysSorted();
   
   bool withinInterval = false;
   float value = 0.0;

   int nextKeyIndex = -1;
   int index;
   real32 x[4];
   real32 y[4];

   // make sure we have something to evaluate
   if( mKeys.empty() )
   {
      return value;
   }

   // check if the time falls into the pre-infinity
   if( time < mKeys.front().mTime )
   {
      if( mPreInfinity == kInfinity_Constant )
      {
         return(mKeys.front().mValue);
      }
      return EvaluateInfinities(time, true, pCache);
   }

   // check if the time falls into the post-infinity
   if( time > mKeys[mKeys.size() - 1].mTime )
   {
      if( mPostInfinity == kInfinity_Constant )
      {
         return(mKeys[mKeys.size() - 1].mValue);
      }
      return EvaluateInfinities(time, false, pCache);
   }

   // check if the animation curve is static
   /*
   if( mIsStatic )
   {
      return(mKeys.front().mValue);
   }
   */

   // check to see if the time falls within the last segment we evaluated
   if( pCache->mLastIndex != -1 )
   {
      if( (pCache->mLastIndex < (mKeys.size() - 1)) && (time > mKeys[pCache->mLastIndex].mTime) )
      {
         nextKeyIndex = pCache->mLastIndex + 1;
         
         if( time == mKeys[nextKeyIndex].mTime )
         {
            pCache->mLastIndex++;
            return mKeys[pCache->mLastIndex].mValue;
         }
         if( time < mKeys[nextKeyIndex].mTime )
         {
            index = pCache->mLastIndex + 1;
            withinInterval = true;
         }
      }
      else if( (pCache->mLastIndex > 0) && (time < mKeys[pCache->mLastIndex].mTime) )
      {
         nextKeyIndex = pCache->mLastIndex - 1;
         if( time > mKeys[nextKeyIndex].mTime )
         {
            index = pCache->mLastIndex;
            withinInterval = true;
         }
         if( time == mKeys[nextKeyIndex].mTime )
         {
            pCache->mLastIndex = nextKeyIndex - 1;
            return mKeys[nextKeyIndex].mValue;
         }
      }
   }

   // it does not, so find the new segment
   if( !withinInterval )
   {
      if( FindKey(time, &index) || (index == 0) )
      {
         //	Exact match or before range of this action,
         //	return exact keyframe value.
         pCache->mLastIndex = index;
         return mKeys[pCache->mLastIndex].mValue;
      }
      else if( index == mKeys.size() )
      {
         // Beyond range of this action return end keyframe value
         pCache->mLastIndex = 0;
         return mKeys[mKeys.size() - 1].mValue;
      }
   }

   // if we are in a new segment, pre-compute and cache the bezier parameters
   if( pCache->mLastInterval != (index - 1) )
   {
      pCache->mLastIndex = index - 1;
      pCache->mLastInterval = pCache->mLastIndex;
      if( close_enough( mKeys[pCache->mLastIndex].mOutTangent, CVector2::Zero() ) )
      {
         pCache->mIsStep = true;
      }
      else
      {
         pCache->mIsStep = false;
         x[0] = mKeys[pCache->mLastIndex].mTime;
         y[0] = mKeys[pCache->mLastIndex].mValue;
         x[1] = x[0] + (mKeys[pCache->mLastIndex].mOutTangent.mX * kOneThird);
         y[1] = y[0] + (mKeys[pCache->mLastIndex].mOutTangent.mY * kOneThird);

         x[3] = mKeys[index].mTime;
         y[3] = mKeys[index].mValue;
         x[2] = x[3] - (mKeys[index].mInTangent.mX * kOneThird);
         y[2] = y[3] - (mKeys[index].mInTangent.mY * kOneThird);

         if( mIsWeighted )
         {
            BezierCreate(pCache, x, y);
         }
         else
         {
            HermiteCreate(pCache, x, y);
         }
      }
   }

   // finally we can evaluate the segment
   if( pCache->mIsStep )
   {
      value = mKeys[pCache->mLastIndex].mValue;
   }
   else if( mIsWeighted )
   {
      value = BezierEvaluate(pCache, time);
   }
   else
   {
      value = HermiteEvaluate(pCache, time);
   }
   return value;
}

//----------------------------------------------------------------------------

//	Description:
//		We want to ensure that (x1, x2) is inside the ellipse
//		(x1^2 + x2^2 - 2(x1 +x2) + x1*x2 + 1) given that we know
//		x1 is within the x bounds of the ellipse.

void constrainInsideBounds( real32 *x1, real32 *x2 )
{
   real32 b, c,  discr,  root;

   if( (*x1 + gkEpsilon32) < kFourThirds )
   {
      b = *x1 - 2.0f;
      c = *x1 - 1.0f;
      discr = sqrtf(b * b - 4 * c * c);
      root = (-b + discr) * 0.5f;
      if( (*x2 + gkEpsilon32) > root )
      {
         *x2 = root - gkEpsilon32;
      }
      else
      {
         root = (-b - discr) * 0.5f;
         if( *x2 < (root + gkEpsilon32) )
         {
            *x2 = root + gkEpsilon32;
         }
      }
   }
   else
   {
      *x1 = kFourThirds - gkEpsilon32;
      *x2 = kOneThird - gkEpsilon32;
   }
}

//	Description:
//
//		Given the bezier curve
//			 B(t) = [t^3 t^2 t 1] * | -1  3 -3  1 | * | 0  |
//									|  3 -6  3  0 |   | x1 |
//									| -3  3  0  0 |   | x2 |
//									|  1  0  0  0 |   | 1  |
//
//		We want to ensure that the B(t) is a monotonically increasing function.
//		We can do this by computing
//			 B'(t) = [3t^2 2t 1 0] * | -1  3 -3  1 | * | 0  |
//									 |  3 -6  3  0 |   | x1 |
//									 | -3  3  0  0 |   | x2 |
//									 |  1  0  0  0 |   | 1  |
//
//		and finding the roots where B'(t) = 0.  If there is at most one root
//		in the interval [0, 1], then the curve B(t) is monotonically increasing.
//
//		It is easier if we use the control vector [ 0 x1 (1-x2) 1 ] since
//		this provides more symmetry, yields better equations and constrains
//		x1 and x2 to be positive.
//
//		Therefore:
//			 B'(t) = [3t^2 2t 1 0] * | -1  3 -3  1 | * | 0    |
//									 |  3 -6  3  0 |   | x1   |
//									 | -3  3  0  0 |   | 1-x2 |
//									 |  1  0  0  0 |   | 1    |
//
//				   = [t^2 t 1 0] * | 3*(3*x1 + 3*x2 - 2)  |
//								   | 2*(-6*x1 - 3*x2 + 3) |
//								   | 3*x1                 |
//								   | 0                    |
//
//		gives t = (2*x1 + x2 -1) +/- sqrt(x1^2 + x2^2 + x1*x2 - 2*(x1 + x2) + 1)
//				  --------------------------------------------------------------
//								3*x1 + 3* x2 - 2
//
//		If the ellipse [x1^2 + x2^2 + x1*x2 - 2*(x1 + x2) + 1] <= 0, (Note
//		the symmetry) x1 and x2 are valid control values and the curve is
//		monotonic.  Otherwise, x1 and x2 are invalid and have to be projected
//		onto the ellipse.
//
//		It happens that the maximum value that x1 or x2 can be is 4/3.
//		If one of the values is less than 4/3, we can determine the
//		boundary constraints for the other value.

void checkMonotonic (real32 *x1, real32 *x2)
{
   real32 d;

   // We want a control vector of [ 0 x1 (1-x2) 1 ] since this provides
   // more symmetry. (This yields better equations and constrains x1 and x2
   // to be positive.)
   *x2 = 1.0f - *x2;

   // x1 and x2 must always be positive
   if( *x1 < 0.0f ) *x1 = 0.0f;
   if( *x2 < 0.0f ) *x2 = 0.0f;

   // If x1 or x2 are greater than 1.0, then they must be inside the
   // ellipse (x1^2 + x2^2 - 2(x1 +x2) + x1*x2 + 1).
   // x1 and x2 are invalid if x1^2 + x2^2 - 2(x1 +x2) + x1*x2 + 1 > 0.0
   if( (*x1 > 1.0) || (*x2 > 1.0) )
   {
      d = *x1 * (*x1 - 2.0f + *x2) + *x2 * (*x2 - 2.0f) + 1.0f;
      if( (d + gkEpsilon32) > 0.0f )
      {
         constrainInsideBounds (x1, x2);
      }
   }

   *x2 = 1.0f - *x2;
}

//	Description:
//		Convert the control values for a polynomial defined in the Bezier
//		basis to a polynomial defined in the power basis (t^3 t^2 t 1).
void bezierToPower( real32 a1, real32 b1, real32 c1, real32 d1,
                    real32 *a2, real32 *b2, real32 *c2, real32 *d2)
{
   real32 a = b1 - a1;
   real32 b = c1 - b1;
   real32 c = d1 - c1;
   real32 d = b - a;
   *a2 = c - b - d;
   *b2 = d + d + d;
   *c2 = a + a + a;
   *d2 = a1;
}

//   Evaluate a polynomial in array form ( value only )
//   input:
//      P               array 
//      deg             degree
//      s               parameter
//   output:
//      ag_horner1      evaluated polynomial
//   process: 
//      ans = sum (i from 0 to deg) of P[i]*s^i
//   restrictions: 
//      deg >= 0           
real32 const ag_horner1(real32 P[], int deg, real32 s)
{
   real32 h = P[deg];
   while( --deg >= 0 ) h = (s * h) + P[deg];
   return(h);
}

typedef struct ag_polynomial
{
   real32 *p;
   int deg;
} AG_POLYNOMIAL;

//	Description
//   Compute parameter value at zero of a function between limits
//       with function values at limits
//   input:
//       a, b      real interval
//       fa, fb    double values of f at a, b
//       f         real valued function of t and pars
//       tol       tolerance
//       pars      pointer to a structure
//   output:
//       ag_zeroin2   a <= zero of function <= b
//   process:
//       We find the zeroes of the function f(t, pars).  t is
//       restricted to the interval [a, b].  pars is passed in as
//       a pointer to a structure which contains parameters
//       for the function f.
//   restrictions:
//       fa and fb are of opposite sign.
//       Note that since pars comes at the end of both the
//       call to ag_zeroin and to f, it is an optional parameter.

real32 const ag_zeroin2(real32 a, real32 b, real32 fa, real32 fb, real32 tol, AG_POLYNOMIAL *pars)
{
   int test;
   real32 c, d, e, fc, del, m, machtol, p, q, r, s;

   // initialization
   machtol = gkEpsilon32;

   // start iteration
label1:
   c = a;  fc = fa;  d = b-a;  e = d;
label2:
   if( fabs(fc) < fabs(fb) )
   {
      a = b;   b = c;   c = a;   fa = fb;   fb = fc;   fc = fa;
   }

   // convergence test
   del = 2.0f * machtol * fabsf(b) + 0.5f*tol;
   m = 0.5f * (c - b);
   test = ((fabsf(m) > del) && (fb != 0.0f));
   if( test )
   {
      if( (fabs(e) < del) || (fabs(fa) <= fabs(fb)) )
      {
         // bisection
         d = m;  e= d;
      }
      else
      {
         s = fb / fa;
         if( a == c )
         {
            // linear interpolation
            p = 2.0f*m*s;    q = 1.0f - s;
         }
         else
         {
            // inverse quadratic interpolation
            q = fa/fc;
            r = fb/fc;
            p = s*(2.0f*m*q*(q-r)-(b-a)*(r-1.0f));
            q = (q-1.0f)*(r-1.0f)*(s-1.0f);
         }
         // adjust the sign
         if( p > 0.0 ) q = -q;
         else p = -p;
         // check if interpolation is acceptable
         s = e;   e = d;
         if( (2.0f*p < 3.0f*m*q-fabs(del*q))&&(p < fabsf(0.5f*s*q)) )
         {
            d = p/q;
         }
         else
         {
            d = m;   e = d;
         }
      }
      // complete step
      a = b;   fa = fb;
      if( fabs(d) > del )   b += d;
      else if( m > 0.0 ) b += del;
      else b -= del;
      fb = ag_horner1 (pars->p, pars->deg, b);
      if( fb*(fc/fabs(fc)) > 0.0 )
      {
         goto label1;
      }
      else
      {
         goto label2;
      }
   }
   return(b);
}

//	Description:
//   Compute parameter value at zero of a function between limits
//   input:
//       a, b            real interval
//       f               real valued function of t and pars
//       tol             tolerance
//       pars            pointer to a structure
//   output:
//       ag_zeroin       zero of function
//   process:
//       Call ag_zeroin2 to find the zeroes of the function f(t, pars).
//       t is restricted to the interval [a, b].
//       pars is passed in as a pointer to a structure which contains
//       parameters for the function f.
//   restrictions:
//       f(a) and f(b) are of opposite sign.
//       Note that since pars comes at the end of both the
//         call to ag_zeroin and to f, it is an optional parameter.
//       If you already have values for fa,fb use ag_zeroin2 directly

real32 const ag_zeroin (real32 a, real32 b, real32 tol, AG_POLYNOMIAL *pars)
{
   real32 fa, fb;

   fa = ag_horner1 (pars->p, pars->deg, a);
   if( fabs(fa) < gkEpsilon32 ) return(a);

   fb = ag_horner1 (pars->p, pars->deg, b);
   if( fabs(fb) < gkEpsilon32 ) return(b);

   return(ag_zeroin2 (a, b, fa, fb, tol, pars));
} 

// Description:
//   Find the zeros of a polynomial function on an interval
//   input:
//       Poly                 array of coefficients of polynomial
//       deg                  degree of polynomial
//       a, b                 interval of definition a < b
//       a_closed             include a in interval (TRUE or FALSE)
//       b_closed             include b in interval (TRUE or FALSE)
//   output: 
//       polyzero             number of roots 
//                            -1 indicates Poly == 0.0
//       Roots                zeroes of the polynomial on the interval
//   process:
//       Find all zeroes of the function on the open interval by 
//       recursively finding all of the zeroes of the derivative
//       to isolate the zeroes of the function.  Return all of the 
//       zeroes found adding the end points if the corresponding side
//       of the interval is closed and the value of the function 
//       is indeed 0 there.
//   restrictions:
//       The polynomial p is simply an array of deg+1 doubles.
//       p[0] is the constant term and p[deg] is the coef 
//       of t^deg.
//       The array roots should be dimensioned to deg+2. If the number
//       of roots returned is greater than deg, suspect numerical
//       instabilities caused by some nearly flat portion of Poly.

int const polyZeroes (real32 Poly[], int deg, real32 a, int a_closed, real32 b, int b_closed, real32 Roots[])
{
   int i, left_ok, right_ok, nr, ndr, skip;
   real32 e, f, s, pe, ps, tol, *p, p_x[22], *d, d_x[22], *dr, dr_x[22];
   AG_POLYNOMIAL ply;

   e = pe = 0.0;  
   f = 0.0;

   for( i = 0 ; i < deg + 1; ++i )
   {
      f += fabs(Poly[i]);
   }
   tol = (fabs(a) + fabs(b))*(deg+1)*gkEpsilon32;

   // Zero polynomial to tolerance?
   if( f <= tol )  return(-1);

   p = p_x;  d = d_x;  dr = dr_x;
   for( i = 0 ; i < deg + 1; ++i )
   {
      p[i] = 1.0f/f * Poly[i];
   }

   // determine true degree
   while( fabs(p[deg]) < tol ) deg--;

   // Identically zero poly already caught so constant fn != 0
   nr = 0;
   if( deg == 0 ) return(nr);

   // check for linear case
   if( deg == 1 )
   {
      Roots[0] = -p[0] / p[1];
      left_ok  = (a_closed) ? (a<Roots[0]+tol) : (a<Roots[0]-tol);
      right_ok = (b_closed) ? (b>Roots[0]-tol) : (b>Roots[0]+tol);
      nr = (left_ok && right_ok) ? 1 : 0;
      if( nr )
      {
         if( a_closed && Roots[0]<a ) Roots[0] = a;
         else if( b_closed && Roots[0]>b ) Roots[0] = b;
      }
      return(nr);
   }
   // handle non-linear case
   else
   {
      ply.p = p;  ply.deg = deg;

      // compute derivative
      for( i=1; i<=deg; i++ ) d[i-1] = i*p[i];

      // find roots of derivative
      ndr = polyZeroes ( d, deg-1, a, 0, b, 0, dr );
      if( ndr == -1 ) return(0);

      // find roots between roots of the derivative
      for( i=skip=0; i<=ndr; i++ )
      {
         if( nr>deg ) return(nr);
         if( i==0 )
         {
            s=a; ps = ag_horner1( p, deg, s);
            if( fabs(ps)<=tol && a_closed ) Roots[nr++]=a;
         }
         else
         {
            s=e; ps=pe;
         }
         if( i==ndr )
         {
            e = b; skip = 0;
         }
         else e=dr[i];
         pe = ag_horner1( p, deg, e );
         if( skip ) skip = 0;
         else
         {
            if( fabs(pe) < tol )
            {
               if( i!=ndr || b_closed )
               {
                  Roots[nr++] = e;
                  skip = 1;
               }
            }
            else if( (ps<0 && pe>0)||(ps>0 && pe<0) )
            {
               Roots[nr++] = ag_zeroin(s, e, 0.0, &ply );
               if( (nr>1) && Roots[nr-2]>=Roots[nr-1]-tol )
               {
                  Roots[nr-2] = (Roots[nr-2]+Roots[nr-1]) * 0.5f;
                  nr--;
               }
            }
         }
      }
   }

   return(nr);
} 

//	Description:
//		Create a constrained single span cubic 2d bezier curve using the
//		specified control points.  The curve interpolates the first and
//		last control point.  The internal two control points may be
//		adjusted to ensure that the curve is monotonic.

void CSpline::BezierCreate(SSplineEvaluationCache * const pCache, real32 x[4], real32 y[4]) const
{
   real32 dx1, dx2, nX1, nX2, oldX1, oldX2;

   real32 rangeX = x[3] - x[0];
   if( rangeX == 0.0f )
   {
      return;
   }

   dx1 = x[1] - x[0];
   dx2 = x[2] - x[0];

   // normalize X control values
   nX1 = dx1 / rangeX;
   nX2 = dx2 / rangeX;

   // if all 4 CVs equally spaced, polynomial will be linear
   if( (nX1 == kOneThird) && (nX2 == kTwoThirds) )
   {
      pCache->mIsLinear = true;
   }
   else
   {
      pCache->mIsLinear = false;
   }

   // save the orig normalized control values
   oldX1 = nX1;
   oldX2 = nX2;

   // check the inside control values yield a monotonic function.
   // if they don't correct them with preference given to one of them.
   //
   // Most of the time we are monotonic, so do some simple checks first
   if( nX1 < 0.0 ) nX1 = 0.0;
   if( nX2 > 1.0 ) nX2 = 1.0;
   if( (nX1 > 1.0) || (nX2 < -1.0) )
   {
      checkMonotonic (&nX1, &nX2);
   }

   // compute the new control points
   if( nX1 != oldX1 )
   {
      x[1] = x[0] + nX1 * rangeX;
      if( oldX1 != 0.0 )
      {
         y[1] = y[0] + (y[1] - y[0]) * nX1 / oldX1;
      }
   }
   if( nX2 != oldX2 )
   {
      x[2] = x[0] + nX2 * rangeX;
      if( oldX2 != 1.0f )
      {
         y[2] = y[3] - (y[3] - y[2]) * (1.0f - nX2) / (1.0f - oldX2);
      }
   }

   // save the control points
   pCache->mX1 = x[0];
   pCache->mX4 = x[3];

   // convert Bezier basis to power basis
   bezierToPower( 0.0f, nX1, nX2, 1.0f,
                 &(pCache->mCoeff[3]), &(pCache->mCoeff[2]), &(pCache->mCoeff[1]), &(pCache->mCoeff[0]) );
   bezierToPower( y[0], y[1], y[2], y[3],
                 &(pCache->mPolyY[3]), &(pCache->mPolyY[2]), &(pCache->mPolyY[1]), &(pCache->mPolyY[0]) );
}

real32 const CSpline::BezierEvaluate(SSplineEvaluationCache * const pCache, real32 const time) const
{
   real32 t, s, poly[4], roots[5];
   int numRoots;

   if( pCache->mX1 == time )
   {
      s = 0.0;
   }
   else if( pCache->mX4 == time )
   {
      s = 1.0;
   }
   else
   {
      s = (time - pCache->mX1) / (pCache->mX4 - pCache->mX1);
   }

   if( pCache->mIsLinear )
   {
      t = s;
   }
   else
   {
      poly[3] = pCache->mCoeff[3];
      poly[2] = pCache->mCoeff[2];
      poly[1] = pCache->mCoeff[1];
      poly[0] = pCache->mCoeff[0] - s;

      numRoots = polyZeroes(poly, 3, 0.0, 1, 1.0, 1, roots);
      if( numRoots == 1 )
      {
         t = roots[0];
      }
      else
      {
         t = 0.0f;
      }
   }
   return(t * (t * (t * pCache->mPolyY[3] + pCache->mPolyY[2]) + pCache->mPolyY[1]) + pCache->mPolyY[0]);
}

//----------------------------------------------------------------------------

void CSpline::HermiteCreate(SSplineEvaluationCache * const pCache, real32 x[4], real32 y[4]) const
{
   real32 tan_x, m1, m2, length, d1, d2;

   // save the control points
   pCache->mX1 = x[0];

   // Compute the difference between the 2 keyframes.					
   real32 const dx = x[3] - x[0];
   real32 const dy = y[3] - y[0];

   //	Compute the tangent at the start of the curve segment.			
   tan_x = x[1] - x[0];
   m1 = m2 = kMaxTan;
   if( tan_x != 0.0f )
   {
      m1 = (y[1] - y[0]) / tan_x;
   }

   tan_x = x[3] - x[2];
   if( tan_x != 0.0f )
   {
      m2 = (y[3] - y[2]) / tan_x;
   }

   length = 1.0f / (dx * dx);
   d1 = dx * m1;
   d2 = dx * m2;
   pCache->mCoeff[0] = (d1 + d2 - dy - dy) * length / dx;
   pCache->mCoeff[1] = (dy + dy + dy - d1 - d1 - d2) * length;
   pCache->mCoeff[2] = m1;
   pCache->mCoeff[3] = y[0];
}

//----------------------------------------------------------------------------

real32 const CSpline::HermiteEvaluate(SSplineEvaluationCache * const pCache, real32 const time) const
{
   real32 t = time - pCache->mX1;
   return(t * (t * (t * pCache->mCoeff[0] + pCache->mCoeff[1]) + pCache->mCoeff[2]) + pCache->mCoeff[3]);
}

//----------------------------------------------------------------------------
