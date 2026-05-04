//----------------------------------------------------------------------------
// Vita ALTIVEC DEFINES
//
// These map PS3 intrinsics to Vita intrinsics. 
//----------------------------------------------------------------------------

#define BP_USE_NEON  1

//----------------------------------------------------------------------------

#ifndef BPE_FORCEINLINE
   #ifndef __cplusplus//soren added to get inline functions to work on PS3
      #define BPE_FORCEINLINE       static inline __attribute__((always_inline))
   #else
      #define BPE_FORCEINLINE       inline __attribute__((always_inline))
   #endif
#endif

//----------------------------------------------------------------------------

// Types
typedef float16x4_t        __vec_half4;
typedef float32x4_t        __vec_float4;
typedef uint8x16_t         __vec_uchar16;
typedef unsigned int       uint32_t;
typedef uint32x4_t         __vec_bool4;
typedef uint32x4_t         __vec_uint4;
typedef int32x4_t          __vec_int4;
typedef uint16x8_t         __vec_ushort8;

#define vec_cast( t, v )      ( t )v

//----------------------------------------------------------------------------

// Defines
#define ENDIAN_INT__BYTE_SIG_0 0
#define ENDIAN_INT__BYTE_SIG_1 1
#define ENDIAN_INT__BYTE_SIG_2 2
#define ENDIAN_INT__BYTE_SIG_3 3

#define ENDIAN_INT__SHORT_SIG_0 0
#define ENDIAN_INT__SHORT_SIG_1 1

//----------------------------------------------------------------------------

// float4 intrinsics mappings
#define vec_xor(a,b)       veorq_u32(a,b)
#define vec_ld(o,p)        vld1q_f32( (const float *)(  (const char*)((void const *)(p)) + (o) ) )
#define vec_madd(a,b,c)    vmlaq_f32(c,a,b)
#define vec_st(src,off,dst) vst1q_f32( (float *)( ((char *) dst ) + off), src )
#define vec_zero(a)        vmovq_n_f32(0.0f)
#define vec_mul(a,b)       vmulq_f32(a,b)
#define vec_nmsub(a,b,res) vmlsq_f32(res,a,b)
#define vec_add(a,b)       vaddq_f32(a,b)
#define vec_sub(a,b)       vsubq_f32(a,b)
#define vec_cmplt(a,b)     vcltq_f32(a,b)
#define vec_cmpgt(a,b)     vcgtq_f32(a,b)
#define vec_cmpge(a,b)     vcgeq_f32(a,b)
#define vec_cmpeq(a,b)     vceqq_f32(a,b)
#define vec_sel(a,b,m)     vbslq_f32(m,b,a)
#define vec_rsqrte(a)      vrsqrteq_f32(a) // rsqrt est
#define vec_re(a)          vrecpeq_f32(a)  // recip est
#define vec_max(a,b)       vmaxq_f32(a,b)
#define vec_min(a,b)       vminq_f32(a,b)
#define vec_dot3(a,b)      sce_vectormath_dot3_f4(a,b)
#define vec_dot3_w0(a,b)   sce_vectormath_dot3_f4(a,b)
#define vec_dot4(a,b)      sce_vectormath_dot4_f4(a,b)
#define vec_cross(a,b)     sce_vectormath_cross_f4(a,b)
#define vec_splat(a,b)     sce_vectormath_splat_f4(a,b)
#define vec_splat_imm(s)   vdupq_n_s32(s) // this one is int!
#define vec_and(a,b)       vec_cast(__vec_float4, vandq_u32((uint32x4_t)a,(uint32x4_t)b))
#define vec_or(a,b)        vorrq_u32(a,b)
#define vec_using_mul()                         // Nothing needed on Vita since we have a mul(a,b)
#define vec_sr(a,b)        vqshlq_u32(a,vqnegq_s32((int32x4_t)b)) // shift right
#define vec_steh(v,slot,s) (*s = vgetq_lane_u32(v,slot)) // store element half
#define vec_clearw(a)      vsetq_lane_f32(0,a,3)

// dodgy
#define vec_splatb(v,s)    vdupq_lane_u8(vreinterpret_u8_f32(vget_low_f32(v)),s)
#define vec_splath(v,s)    vdupq_lane_u16(vget_low_u16(v),s)

//-----------------------------------------------------------------------------

// half4 intrinsics mappings
#define hvec_ld(o,p)          vld1_f16( (const __fp16 *)(  (const char*)((void const *)(p)) + (o) ) )
#define hvec_st(src,off,dst)  vst1_f16( (__fp16 *)( ((char *) dst ) + off), src )
#define vec_to_hvec(a)        vcvt_f16_f32(a)   
#define hvec_to_vec(a)        vcvt_f32_f16(a)   

//-----------------------------------------------------------------------------

#define     sce_vectormath_mul_f4(a, b)         vmulq_f32(a,b)
#define     sce_vectormath_nmsub_f4(a, b, c)    vmlsq_f32(c,a,b)

//-----------------------------------------------------------------------------
// Perm replacements
//-----------------------------------------------------------------------------

BPE_FORCEINLINE
float32x4_t sce_vectormath_xyzz_f4(float32x4_t xyzw)
{
   return vcombine_f32( vget_low_f32(xyzw), vdup_lane_f32( vget_high_f32(xyzw), 0 ) );
}

//-----------------------------------------------------------------------------

BPE_FORCEINLINE
float32x4_t sce_vectormath_yzxy_f4(float32x4_t xyzw)
{
   float32x2_t xy = vget_low_f32( xyzw );
   float32x2_t yz = vext_f32( xy, vget_high_f32( xyzw ), 1 );
   float32x4_t yzxy = vcombine_f32( yz, xy );
   return yzxy;
}

//-----------------------------------------------------------------------------

BPE_FORCEINLINE
float32x4_t sce_vectormath_zxyz_f4(float32x4_t xyzw)
{
   float32x4_t xyzz = vcombine_f32( vget_low_f32( xyzw ), vdup_lane_f32( vget_high_f32( xyzw ), 0 ) );
   float32x4_t zxyz = vextq_f32( xyzz, xyzz, 3 );
   return zxyz;
}

//-----------------------------------------------------------------------------
// Misc
//-----------------------------------------------------------------------------

BPE_FORCEINLINE
float32x4_t sce_vectormath_splat_f4(float32x4_t v, uint32_t slot)
{
   switch ( slot )
   {
   case	0:	return vdupq_lane_f32( vget_low_f32( v ) , 0 ); 
   case	1:	return vdupq_lane_f32( vget_low_f32( v ) , 1 ); 
   case	2:	return vdupq_lane_f32( vget_high_f32( v ) , 0 );
   default:	return vdupq_lane_f32( vget_high_f32( v ) , 1 );
   }
}

//-----------------------------------------------------------------------------

BPE_FORCEINLINE 
float32x4_t sce_vectormath_dot3_f4(float32x4_t a, float32x4_t b)
{
   float32x4_t dot = vmulq_f32(a,b);
   float32x2_t xy = vget_low_f32( dot );
   float32x2_t zw = vget_high_f32( dot );
   float32x2_t xPlusY = vpadd_f32( xy , xy );
   float32x2_t xPlusYPlusZ = vadd_f32( xPlusY, zw );
   return vdupq_lane_f32( xPlusYPlusZ, 0 );
}

//-----------------------------------------------------------------------------

BPE_FORCEINLINE
float32x4_t sce_vectormath_cross_f4(float32x4_t a, float32x4_t b)
{
   float32x4_t yzxy = sce_vectormath_yzxy_f4(a);
   float32x4_t cabc = sce_vectormath_zxyz_f4(b);
   float32x4_t zxyz = sce_vectormath_zxyz_f4(a);
   float32x4_t bcab = sce_vectormath_yzxy_f4(b);
   float32x4_t result = sce_vectormath_mul_f4(yzxy, cabc);
   result = sce_vectormath_nmsub_f4(zxyz, bcab, result);
   return result;
}

//-----------------------------------------------------------------------------

BPE_FORCEINLINE 
__vec_float4 vec_get_all_x(__vec_float4 a, __vec_float4 b, __vec_float4 c, __vec_float4 d)
{
   float32x4x2_t _p = vzipq_f32(a, b);
   float32x4x2_t _q = vzipq_f32(c, d);
   float32x4x2_t _r = vzipq_f32(_p.val[0], _q.val[0]);
   return _r.val[0];
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
__vec_int4 vec_cts(__vec_float4 a, uint32_t fracbits)
{
   // note fracbits != 0 is not supported yet
   ASSERT(fracbits == 0);
   return vcvtq_s32_f32(a);
}

//----------------------------------------------------------------------------
// Comparison
//----------------------------------------------------------------------------

BPE_FORCEINLINE 
int vec_all_le( const __vec_float4 a, const __vec_float4 b )
{
   uint32x4_t le    = vcleq_f32(a,b);
   uint16x4_t res16 = vmovn_u32(le);
   uint64x1_t res64 = vreinterpret_u64_u16(res16);
   return(res64[0] == 0xffffffffffffffffULL);
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
int vec_all_ge( const __vec_float4 a, const __vec_float4 b )
{
   uint32x4_t ge    = vcgeq_f32(a,b);
   uint16x4_t res16 = vmovn_u32(ge);
   uint64x1_t res64 = vreinterpret_u64_u16(res16);
   return(res64[0] == 0xffffffffffffffffULL);
}

//----------------------------------------------------------------------------

// Returns 1 if xyz bounds overlap
BPE_FORCEINLINE 
int vec_bounds_overlap( const __vec_float4 bound_min0, const __vec_float4 bound_max0,
                        const __vec_float4 bound_min1, const __vec_float4 bound_max1 )
{
   uint32x4_t le0   = vcleq_f32(bound_min0, bound_max1);
   uint32x4_t le1   = vcleq_f32(bound_min1, bound_max0);
   uint32x4_t le    = vandq_u32(le0, le1);
   uint32x2_t xy    = vget_low_u32(le);
   uint32x2_t zz    = vdup_lane_u32(vget_high_u32(le), 0);
   uint32x2_t res32 = vand_u32(xy, zz);
   uint64x1_t res64 = vreinterpret_u64_u32(res32);
   return(res64[0] == 0xffffffffffffffffULL);
}

//----------------------------------------------------------------------------

// Returns 1 if xyz bounds overlap
BPE_FORCEINLINE 
int vec_bounds_overlap_xyzw( const __vec_float4 bound_min0, const __vec_float4 bound_max0,
                             const __vec_float4 bound_min1, const __vec_float4 bound_max1 )
{
   uint32x4_t le0   = vcleq_f32(bound_min0, bound_max1);
   uint32x4_t le1   = vcleq_f32(bound_min1, bound_max0);
   uint32x4_t le    = vandq_u32(le0, le1);
   uint16x4_t res16 = vmovn_u32(le);
   uint64x1_t res64 = vreinterpret_u64_u16(res16);
   return(res64[0] == 0xffffffffffffffffULL);
}

//----------------------------------------------------------------------------
