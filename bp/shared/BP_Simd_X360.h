//----------------------------------------------------------------------------
// X360 ALTIVEC DEFINES
//
// These map PS3 intrinsics to X360 intrinsics. 
// Only vec_all_le is not directly supported with a 1:1 replacement
//----------------------------------------------------------------------------

#define BP_USE_ALTIVEC  1

//----------------------------------------------------------------------------

#ifndef BPE_FORCEINLINE
#define BPE_FORCEINLINE       __forceinline
#endif

//----------------------------------------------------------------------------

// Types
typedef __vector4 __vec_int4;
typedef __vector4 __vec_uint4;
typedef __vector4 __vec_bool4;               // X360 only has 1 type
typedef __vector4 __vec_float4;
typedef __vector4 __vec_short8;
typedef __vector4 __vec_uchar16;


//----------------------------------------------------------------------------

// Defines
#define vec_cast( t, v )   ( v )

#define ENDIAN_INT__BYTE_SIG_0 3
#define ENDIAN_INT__BYTE_SIG_1 2
#define ENDIAN_INT__BYTE_SIG_2 1
#define ENDIAN_INT__BYTE_SIG_3 0

#define ENDIAN_INT__SHORT_SIG_0 1
#define ENDIAN_INT__SHORT_SIG_1 0

//----------------------------------------------------------------------------

// Intrinsics mappings
#define vec_ld(a,b)        __lvlx(b,a)
#define vec_st(a,b,c)      __stvlx(a,c,b)
#define vec_zero(a)        __vzero()         // Using reg param for PS3
#define vec_mul(a,b)       __vmulfp(a,b)
#define vec_madd(a,b,c)    __vmaddfp(a,b,c)
#define vec_nmsub(a,b,c)   __vnmsubfp(a,b,c)
#define vec_add(a,b)       __vaddfp(a,b)
#define vec_sub(a,b)       __vsubfp(a,b)
#define vec_splat(a,b)     __vspltw(a,b)
#define vec_cmplt(a,b)     __vcmpgtfp(b,a)
#define vec_cmpgt(a,b)     __vcmpgtfp(a,b)
#define vec_cmpge(a,b)     __vcmpgefp(a,b)
#define vec_cmpeq(a,b)     __vcmpeqfp(a,b)
#define vec_sel(a,b,c)     __vsel(a,b,c)
#define vec_rsqrte(a)      __vrsqrtefp(a)
#define vec_re(a)          __vrefp(a)
#define vec_max(a,b)       __vmaxfp(a,b)
#define vec_min(a,b)       __vminfp(a,b)
#define vec_xor(a,b)       __vxor(a,b)
#define vec_dot3(a,b)      __vmsum3fp(a,b)
#define vec_dot3_w0(a,b)   __vmsum3fp(a,b)
#define vec_dot4(a,b)      __vmsum4fp(a,b)
#define vec_splat_imm(s)   __vspltisw(s)
#define vec_splatb(a,b)    __vspltb(a,b)
#define vec_splath(a,b)    __vsplth(a,b)
#define vec_sr(a,b)        __vsrw(a,b)
#define vec_and(a,b)       __vand(a,b)
#define vec_or(a,b)        __vor(a,b)
#define vec_cts(a,b)       __vctsxs(a,b)
#define vec_steh(a,b,c)    __stvehx(a,c,b)
#define vec_using_mul()                         // Nothing needed on X360 since we have a mul(a,b)
#define vec_perm(a,b,c)    __vperm(a,b,c)

//----------------------------------------------------------------------------

// X360 does not support vec_all_?, vec_any_? comparisons, but we can use CR to check results.
// NOTE: These functions return either 0, (1<<5) or (1<<7)

BPE_FORCEINLINE 
const unsigned int vec_all_le( const __vec_float4 a, const __vec_float4 b )
{
   unsigned int CR;
   __vcmpgefpR( b, a, &CR );  // (1<<7) = all (b >= a),  (1<<5) = none (b >= a)
   return CR & (1<<7);
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
const unsigned int vec_all_ge( const __vec_float4 a, const __vec_float4 b )
{
   unsigned int CR;
   __vcmpgefpR( a, b, &CR );  // (1<<7) = all (a >= b),  (1<<5) = none (a >= b)
   return CR & (1<<7);
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE
const unsigned int vec_any_lt( const __vec_float4 a, const __vec_float4 b )
{
   unsigned int CR;
   __vcmpgtfpR( a, b, &CR );  // (1<<7) = all (a > b), (1<<5) = none (a > b)
   return( (~CR) & (1<<7) );
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE
const unsigned int vec_all_in( const __vec_float4 a, const __vec_float4 b )
{
   unsigned int CR;
   __vcmpbfpR( a, b, &CR );  // (1<<5) = if all:  b > a > -b
   return( CR & (1<<5) );
}

//----------------------------------------------------------------------------

