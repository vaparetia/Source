//----------------------------------------------------------------------------
// bp_math.c
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include "bp_math.h"
#include "bp_vector.h"
#include "bp_matrix.h"
#include "bp_bound.h"
#include "bp_quaternion.h"
#include "bp_color.h"
#include "bp_curve.h"
#include "BP_Asm.h"
#include "bp_vu.h"
#include "bp_ee.h"

//----------------------------------------------------------------------------
// DATA
//----------------------------------------------------------------------------

// CPU register storage
//ALIGN16_PRE VU0      gVU0 ALIGN16_POST = {0};
//ALIGN16_PRE EE       gEE  ALIGN16_POST = {0};

// Bezier/CatmullRom curve vector storage
ALIGN16_PRE FVECTOR  gCurveVec[10] ALIGN16_POST = {0};

//----------------------------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------------------------

#if BP_VITA
float32x4x4_t gkUnitMatrix = 
{ 
   (float32x4_t) { 1.0f, 0.0f, 0.0f, 0.0f },
   (float32x4_t) { 0.0f, 1.0f, 0.0f, 0.0f },
   (float32x4_t) { 0.0f, 0.0f, 1.0f, 0.0f },
   (float32x4_t) { 0.0f, 0.0f, 0.0f, 1.0f } 
};
#endif

void BP_InitMath()
{
// Initialize floating point exceptions
#ifdef STEVEB
   /*
   Setting the bit on, masks out the exception.
   Only use the first parameter, the second parm 
   is a mask with all bits set for the exception 
   bit field.

   _EM_INVALID =0x00000010
   _EM_DENORMAL = 0x00080000
   _EM_ZERODIVIDE = 0x00000008
   _EM_OVERFLOW = 0x00000004
   _EM_UNDERFLOW = 0x00000002
   _EM_INEXACT 0x00000001
   */

   #if defined(BP_WIN32)
   _control87 (~(/*_EM_OVERFLOW |*/ _EM_ZERODIVIDE /*| _EM_INVALID*/), _MCW_EM);
   #endif

   #if defined(BP_360)
   //Commented out since it slows down game considerably
   //_clearfp();
   //_controlfp (~(/*EM_INEXACT | _EM_UNDERFLOW | _EM_OVERFLOW |*/ _EM_ZERODIVIDE /*| _EM_INVALID*/), _MCW_EM);
   #endif

#endif   //#ifdef STEVEB

   // Initialize CPU registers that are constants
//   vu_reset();
//   ee_reset();
}

//----------------------------------------------------------------------------

// returns TRUE if "f" is a valid float or FALSE if invalid
int BP_Float_Check( const float f )
{
   // Lookup int value
   const int i = *(const int*)&f;

/*
   // Max?
   if( f == FLT_MAX )
   {
      bp_math_break;
      return FALSE;
   }

   // Min?
   if( f == -FLT_MAX )
   {
      bp_math_break;
      return FALSE;
   }
*/

   // Invalid float?
   if( (i & 0x7f800000) == 0x7f800000 )
   {
      bp_math_break;
      return FALSE;
   }

#if BPE_IS_ENDIAN_BIG()
   //unbyteswapped 1.0?
   if( i == 0x0000803f )
   {
      bp_math_break;
      return FALSE;
   }
#endif

   return TRUE;
}

//----------------------------------------------------------------------------

// Original PS2 inverse function. C commented ASM version
//P:\PS2\SDK\tools_libs-3.0.2-4\sce\ee\src\lib\vu0\libvu0.c(306):void sceVu0InversMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
#if 0
void sceVu0InversMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
#ifdef SCE_VU0_SAFE_CONTEXT
   int ret = DI();
#endif
   __asm__ __volatile__("\n\
                           //        x         y         z         w
   lq $8,0x0000(%1)\n\     //$8  = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] }
   lq $9,0x0010(%1)\n\     //$9  = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] }
   lq $10,0x0020(%1)\n\    //$10 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] }
   lqc2 vf4,0x0030(%1)\n\  //vf4 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] }
   \n\
   vmove.xyzw vf5,vf4\n\                     //vf5 = { m1[3][0]   ,  m1[3][1] ,  m1[3][2] ,  m1[3][3] }
   vsub.xyz vf4,vf4,vf4		#vf4.xyz=0;\n\    //vf4 = { 0          ,  0        ,  0        ,  m1[3][3] }
   vmove.xyzw vf9,vf4\n\                     //vf9 = { 0          ,  0        ,  0        ,  m1[3][3] }
   qmfc2    $11,vf4\n\                       //$11 = { 0          ,  0        ,  0        ,  m1[3][3] }
   \n\
   #“]’u\n\                                                                                        //   x         y           z         w
   pextlw     $12,$9,$8\n\    //$12.x = $8.x,  $12.y = $9.x,  $12.z = $8.y,  $12.w = $9.y       $12 = { m1[0][0], m1[1][0],   m1[0][1], m1[1][1]   }
   pextuw     $13,$9,$8\n\    //$13.x = $8.z,  $13.y = $9.z,  $13.z = $8.w,  $13.w = $9.w       $13 = { m1[0][2], m1[1][2],   m1[0][3], m1[1][3]   }
   pextlw     $14,$11,$10\n\  //$14.x = $10.x, $14.y = $11.x, $14.z = $10.y, $14.w = $11.y      $14 = { m1[2][0], 0,          m1[2][1], 0          }
   pextuw     $15,$11,$10\n\  //$15.x = $10.z, $15.y = $11.z, $15.z = $10.w, $15.w = $11.w      $15 = { m1[2][2], 0,          m1[2][3], m1[3][3]   }

   pcpyld     $8,$14,$12\n\   //$8.x  = $12.x, $8.y  = $12.y, $8.z  = $14.x, $8.w  = $14.y      $8  = { m1[0][0], m1[1][0],   m1[2][0], 0          }
   pcpyud     $9,$12,$14\n\   //$9.x  = $12.z, $9.y  = $12.w, $9.z  = $14.z, $9.w  = $14.w      $9  = { m1[0][1], m1[1][1],   m1[2][1], 0          } 
   pcpyld     $10,$15,$13\n\  //$10.x = $13.x, $10.y = $13.y, $10.z = $15.x, $10.w = $15.y      $10 = { m1[0][2], m1[1][2],   m1[2][2], 0          }
   \n\
   qmtc2    $8,vf6\n\         //vf6 = { m1[0][0], m1[1][0],   m1[2][0], 0          }
   qmtc2    $9,vf7\n\         //vf7 = { m1[0][1], m1[1][1],   m1[2][1], 0          }
   qmtc2    $10,vf8\n\        //vf8 = { m1[0][2], m1[1][2],   m1[2][2], 0          }
   \n\
   #“àÏ\n\
   vmulax.xyz	ACC,   vf6,vf5\n\    //       ACC  = { m1[0][0], m1[1][0], m1[2][0], 0 } * m1[3][0] 
   vmadday.xyz	ACC,   vf7,vf5\n\    //       ACC += { m1[0][1], m1[1][1], m1[2][1], 0 } * m1[3][1]
   vmaddz.xyz	vf4,vf8,vf5\n\       // vf4 = ACC +  { m1[0][2], m1[1][2], m1[2][2], 0 } * m1[3][3]
   vsub.xyz	vf4,vf9,vf4\n\          // vf4.xyz = -vf4.xyz;  vf4.w = m1[3][3]
   \n\
   sq $8,0x0000(%0)\n\
   sq $9,0x0010(%0)\n\
   sq $10,0x0020(%0)\n\
   sqc2 vf4,0x0030(%0)\n\
   ": : "r" (m0) , "r" (m1):"$8","$9","$10","$11","$12","$13","$14","$15", "memory");
#ifdef SCE_VU0_SAFE_CONTEXT
   if (ret) EI();
#endif
}

#endif


//----------------------------------------------------------------------------
// Floor/Ceil functions. TODO: Make inline once collision is stable
//----------------------------------------------------------------------------

float BP_Float_Floor( const float a )
{
   return floorf(a);
}

//----------------------------------------------------------------------------

float BP_Float_Ceil( const float a )
{
#ifdef BP_USE_PS2_CEIL
   const int   one_i = 0x3f7fffff;
   const float one_f = *(const float*)&one_i;
   const float b = a + one_f;
   const int   c = (int)b;
   const float d = (float)c;
   return d;
#else
   return ceilf(a);
#endif
}

//----------------------------------------------------------------------------

void BP_Vec4_CeilVec( FVECTOR *r, const FVECTOR *a )
{
   r->vx = BP_Float_Ceil( a->vx );
   r->vy = BP_Float_Ceil( a->vy );
   r->vz = BP_Float_Ceil( a->vz );
   r->vw = BP_Float_Ceil( a->vw );
}

//----------------------------------------------------------------------------

void BP_Vec4_CeilIVec( IVECTOR *r, const FVECTOR *a )
{
   r->vx = (int)BP_Float_Ceil( a->vx );
   r->vy = (int)BP_Float_Ceil( a->vy );
   r->vz = (int)BP_Float_Ceil( a->vz );
   r->vw = (int)BP_Float_Ceil( a->vw );
}

//----------------------------------------------------------------------------

static inline long _fast_floor(float val)
{
   // doesn't matter for the sin/cos code if floor fails when (int)(-q) == -q
   return val < 0 ? (int)(val)-1 : (int)(val);
}

static inline float _fast_mod2pi(float angle)
{
   // approximations are valid from -pi to pi
   static float const sk2Pi   = 2.0f*PI;
   static float const skOO2Pi = 1.0f/(2.0f*PI);
   return angle - _fast_floor(angle*skOO2Pi)*sk2Pi;
}

static inline float _sinSeg( float angle )
{
   // verified |s| <= 1.0f
   float x2 = angle*angle;
   // sin(x) ~ x - (1/3!) * x^3 + (1/5!) * x^5 - (1/7!) * x^7 + (1/9!) * x^9
   // error plot: http://fooplot.com/x-(1/6)*x^3+(1/120)*x^5-(1/5040)*x^7+(1/362880)*x^9-sin(x)
   float s = angle*(0.9999975f + x2*(-1.0f/6.0f + x2*(1.0f/120.0f + x2*(-1.0f/5040.0f + x2*(1.0f/362880.0f)))));
   return s;
}

static inline float _cosSeg( float angle )
{
   float x2 = angle*angle;
   // cos(x) ~ 1 - (1/2!) * x^2 + (1/4!) * x^4 - (1/6!) * x^6 + (1/8!) * x^8
   // error plot: http://fooplot.com/1-(1/2)*x^2+(1/24)*x^4-(1/720)*x^6+(1/40320)*x^8-cos(x)
   float c = 1.0f + x2*(-1.0f/2.0f + x2*(1.0f/24.0f + x2*(-1.0f/720.0f + x2*(1.0f/40320.0f))));
   return c;
}

#define USE_SIN_COS_APPROXIMATION 1

/* ---------------------------------------------------------------- */

void BP_SinCosFast( FVECTOR *res, float angle )
{
#if USE_SIN_COS_APPROXIMATION
   float s, c;
   // cast into range -PI/2 -> PI/2 for much better accuracy
   if (angle < -PI/2)
   {
      s = _sinSeg(-PI - angle);
      // on VITA this is faster than c = BP_Sqrt(1-s*s)
      // also more accurate, and safer
      // win win win!
      c = -_cosSeg(-PI - angle);
   }
   else if (angle > PI/2)
   {
      s = _sinSeg(PI - angle);
      c = -_cosSeg(PI - angle);
   }
   else
   {
      s = _sinSeg(angle);
      c = _cosSeg(angle);
   }
   bp_math_assert( angle >= -(PI + DELTA) && angle <= (PI + DELTA) );
   bp_math_assert( BP_Fabsf(s) <= 1.0f && BP_Fabsf(c) <= 1.0f );
   res->vx = s;
   res->vy = c;
#else
   res->vx = sinf(angle);
   res->vy = cosf(angle);
#endif
}

void BP_SinCos( FVECTOR *res, float angle )
{
#if USE_SIN_COS_APPROXIMATION
   // get in range
   angle = _fast_mod2pi(angle + PI) - PI;
#endif
   BP_SinCosFast(res, angle);
}

/* ---------------------------------------------------------------- */

float BP_SinFast( float angle )
{
#if USE_SIN_COS_APPROXIMATION
   float s;
   if (angle < -PI/2)
   {
      s = _sinSeg(-PI - angle);
   }
   else if (angle > PI/2)
   {
      s = _sinSeg(PI - angle);
   }
   else
   {
      s = _sinSeg(angle);
   }
   bp_math_assert( angle >= -(PI + DELTA) && angle <= (PI + DELTA) );
   bp_math_assert( BP_Fabsf(s) <= 1.0f );
   return s;
#else
   return sinf(angle);
#endif
}

float BP_Sin( float angle )
{
#if USE_SIN_COS_APPROXIMATION
   // get in range
   angle = _fast_mod2pi(angle + PI) - PI;
#endif
   return BP_SinFast(angle);
}

/* ---------------------------------------------------------------- */

float BP_CosFast( float angle )
{
#if USE_SIN_COS_APPROXIMATION
   float c;
   if (angle < -PI/2)
   {
      c = -_cosSeg(-PI - angle);
   }
   else if (angle > PI/2)
   {
      c = -_cosSeg(PI - angle);
   }
   else
   {
      c = _cosSeg(angle);
   }
   bp_math_assert( angle >= -(PI + DELTA) && angle <= (PI + DELTA) );
   bp_math_assert( BP_Fabsf(c) <= 1.0f );
   return c;
#else
   return cosf(angle);
#endif
}

float BP_Cos( float angle )
{
#if USE_SIN_COS_APPROXIMATION
   // get in range
   angle = _fast_mod2pi(angle + PI) - PI;
#endif
   return BP_CosFast(angle);
}

/* ---------------------------------------------------------------- */

static inline float _asinLo( float x )
{
   float x2 = x*x;
   // asin(x) ~ x + (1/6) * x^3 + (3/40) * x^5 + (5/112) * x^7
   // error plot: http://www.fooplot.com/x+x^3/6+3*x^5/40+5*x^7/112-asin(x)
   float a = x*(1.0f + x2*(1.0f/6.0f + x2*(3.0f/40.0f + x2*(5.0f/112.0f))));
   return a;
}

static inline float _asinHi( float x )
{
   if (x > 1)
   {
      // for safety
      // VITA isn't too bad about branches
      return PI/2;
   }
   // for |x| > 0.5, asin(x) = pi/2 - 2 asin(sqrt((1-x)/2))
   x = PI/2 - 2*_asinLo(BP_Sqrt((1-x)/2));
   return x;
}

float BP_ASin( float x )
{
   float a;
   if (x < -0.5f)
   {
      a = -_asinHi(-x);
   }
   else if (x > 0.5f)
   {
      a = _asinHi(x);
   }
   else
   {
      a = _asinLo(x);
   }
   return a;
}

// see:
// http://www.dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
float BP_ATan2Approx( float y, float x )
{
   // alternative values pulled from http://www.vlfeat.org/api/mathop_8h.html
   // checked against a range of random values
   // c3 = 0.1821f, c1 = -0.9675f: average error 0.0038, max error 0.0061 (radians)
   // c3 = 0.1963f, c1 = -0.9817f: average error 0.0034, max error 0.0101
   // chose the values with the lower max error
   static float const skC3 =  0.1821f;
   static float const skC1 = -0.9675f;

   float angle;
   float abs_y = BP_Fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition

   if (x >= 0)
   {
      float r = (x - abs_y) / (x + abs_y);
      angle = (skC3*r*r + skC1)*r + PI/4;
   }
   else
   {
      float r = (abs_y + x) / (abs_y - x);
      angle = (skC3*r*r + skC1)*r + 3*PI/4;
   }

   // negate if in quad III or IV
   if (y < 0)
   {
      angle = -angle;
   }
   return angle;
}