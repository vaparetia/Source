//----------------------------------------------------------------------------
// BP_Simd_PS3.h
//----------------------------------------------------------------------------

#define BP_USE_ALTIVEC  1

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
typedef vector int            __vec_int4;
typedef vector bool int       __vec_bool4;
typedef vector float          __vec_float4;
typedef vector unsigned int   __vec_uint4;
typedef vector short          __vec_short8;
typedef vector unsigned short __vec_ushort8;
typedef vector unsigned char  __vec_uchar16;

#define vec_cast( t, v )      ( t )v

//----------------------------------------------------------------------------

// Defines
#define ENDIAN_INT__BYTE_SIG_0 3
#define ENDIAN_INT__BYTE_SIG_1 2
#define ENDIAN_INT__BYTE_SIG_2 1
#define ENDIAN_INT__BYTE_SIG_3 0

#define ENDIAN_INT__SHORT_SIG_0 1
#define ENDIAN_INT__SHORT_SIG_1 0

//----------------------------------------------------------------------------

// Intrinsics mappings
#define vec_mul(a,b)       vec_madd(a,b,zero)         // There is no mul(a,b) on PS3
#define vec_zero(a)        vec_xor(a,a)               // There is no vec_zero() on PS3
#define vec_dot3(a,b)      ps3_vec_dot3(a,b,zero)     // There is no dot3(a,b) on PS3
#define vec_dot3_w0(a,b)   ps3_vec_dot4(a,b,zero)     // dot4(a,b) is faster than dot3(a,b) for PS3
#define vec_dot4(a,b)      ps3_vec_dot4(a,b,zero)     // There is no dot4(a,b) on PS3
#define vec_splat_imm(s)   vec_splats(s);
#define vec_splatb(a,b)    vec_splat( ( vector char )a,b)
#define vec_splath(a,b)    vec_splat( ( vector short )a,b)
#define vec_steh(a,b,c)    vec_ste((__vec_short8)(a),b,c)

// PS3 only has vec_madd(a,b) so we need to use vec_madd(a,b,zero). 
// This macro conveniently declares zero
#define vec_using_mul()    \
   __vec_float4 zero;      \
   zero = vec_zero(zero)

//----------------------------------------------------------------------------

// PS3 does not directly support vec_dot3(a,b) so we have to hand compute :( 
BPE_FORCEINLINE 
const __vec_float4 ps3_vec_dot3( const __vec_float4 a,
                                 const __vec_float4 b,
                                 const __vec_float4 zero )
{
   __vec_float4 dot, tmp_y, tmp_z;
   dot = vec_madd(a, b, zero);
   tmp_y = vec_splat(dot, 1);
   tmp_z = vec_splat(dot, 2);
   dot = vec_add(dot, tmp_y);
   dot = vec_add(dot, tmp_z);
   dot = vec_splat(dot, 0);
   return dot;
}

//----------------------------------------------------------------------------

// PS3 does not directly support vec_dot4(a,b) so we have to hand compute :( 
// This is actually an instruction less than "ps3_vec_dot3" :)
BPE_FORCEINLINE
const __vec_float4 ps3_vec_dot4( const __vec_float4 a, const __vec_float4 b, const __vec_float4 zero )
{
   __vec_float4 dot, tmp;
   dot = vec_madd(a, b, zero);
   tmp = vec_sld(dot, dot, 4);
   dot = vec_add(dot, tmp);
   tmp = vec_sld(dot, dot, 8);
   dot = vec_add(dot, tmp);
   return dot;
}

//----------------------------------------------------------------------------

