//-----------------------------------------------------------------------------
// bp_ee.h
//-----------------------------------------------------------------------------
#ifndef __BP_EE_H__
#define __BP_EE_H__

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------
#include "bp_vector.h"

//----------------------------------------------------------------------------
// ASSERT defines
//----------------------------------------------------------------------------

// Enable/disable ee asserts
#if defined(_DEBUG) && defined(STEVEB)
#define BP_USE_EE_ASSERT
#endif

// Declare ee break/assert
#ifdef BP_USE_EE_ASSERT
#define bp_ee_break               BP_BREAK
#define bp_ee_assert(__exp__)     if(!(__exp__)) bp_ee_break
#else
#define bp_ee_break
#define bp_ee_assert(__exp__)
#endif


//-----------------------------------------------------------------------------
// Register defines
//-----------------------------------------------------------------------------

#define gpr0            (&pPS2->ee.gpr[0])
#define gpr1            (&pPS2->ee.gpr[1])
#define gpr2            (&pPS2->ee.gpr[2])
#define gpr3            (&pPS2->ee.gpr[3])
#define gpr4            (&pPS2->ee.gpr[4])
#define gpr5            (&pPS2->ee.gpr[5])
#define gpr6            (&pPS2->ee.gpr[6])
#define gpr7            (&pPS2->ee.gpr[7])
#define gpr8            (&pPS2->ee.gpr[8])
#define gpr9            (&pPS2->ee.gpr[9])
#define gpr10           (&pPS2->ee.gpr[10])
#define gpr11           (&pPS2->ee.gpr[11])

#define gpr0_int        (&pPS2->ee.gpr[0].vx)
#define gpr1_int        (&pPS2->ee.gpr[1].vx)
#define gpr2_int        (&pPS2->ee.gpr[2].vx)
#define gpr3_int        (&pPS2->ee.gpr[3].vx)
#define gpr4_int        (&pPS2->ee.gpr[4].vx)
#define gpr5_int        (&pPS2->ee.gpr[5].vx)
#define gpr6_int        (&pPS2->ee.gpr[6].vx)
#define gpr7_int        (&pPS2->ee.gpr[7].vx)
#define gpr8_int        (&pPS2->ee.gpr[8].vx)
#define gpr9_int        (&pPS2->ee.gpr[9].vx)
#define gpr10_int       (&pPS2->ee.gpr[10].vx)
#define gpr11_int       (&pPS2->ee.gpr[11].vx)

#define gpr0_ptr        ((void*)pPS2->ee.gpr[0].vx)
#define gpr1_ptr        ((void*)pPS2->ee.gpr[1].vx)
#define gpr2_ptr        ((void*)pPS2->ee.gpr[2].vx)
#define gpr3_ptr        ((void*)pPS2->ee.gpr[3].vx)
#define gpr4_ptr        ((void*)pPS2->ee.gpr[4].vx)
#define gpr5_ptr        ((void*)pPS2->ee.gpr[5].vx)
#define gpr6_ptr        ((void*)pPS2->ee.gpr[6].vx)
#define gpr7_ptr        ((void*)pPS2->ee.gpr[7].vx)
#define gpr8_ptr        ((void*)pPS2->ee.gpr[8].vx)
#define gpr9_ptr        ((void*)pPS2->ee.gpr[9].vx)
#define gpr10_ptr       ((void*)pPS2->ee.gpr[10].vx)
#define gpr11_ptr       ((void*)pPS2->ee.gpr[11].vx)

#define fpr00           (&pPS2->ee.fpr[0])
#define fpr01           (&pPS2->ee.fpr[1])
#define fpr02           (&pPS2->ee.fpr[2])
#define fpr03           (&pPS2->ee.fpr[3])
#define fpr04           (&pPS2->ee.fpr[4])
#define fpr05           (&pPS2->ee.fpr[5])
#define fpr06           (&pPS2->ee.fpr[6])
#define fpr07           (&pPS2->ee.fpr[7])
#define fpr08           (&pPS2->ee.fpr[8])
#define fpr09           (&pPS2->ee.fpr[9])


//-----------------------------------------------------------------------------
// Branching macros
//-----------------------------------------------------------------------------

// General purpose registers are only 64 bit so only compare x and y components
#define ee_bne( _a, _b, _label )       if( ((_a)->vx != (_b)->vx) || ((_a)->vy != (_b)->vy) ) goto _label
#define ee_bne_int( _a, _b, _label )   if( (*((const int*)_a)) != (_b)->vx ) goto _label
#define ee_beq( _a, _b, _label )       if( (((const IVECTOR*)_a)->vx == (_b)->vx) && (((const IVECTOR*)_a)->vy == (_b)->vy) ) goto _label
#define ee_beq_int( _a, _b, _label )   if( (*((const int*)_a)) == (_b)->vx ) goto _label
#define ee_bltz( _a, _label )          if( ((_a)->vx < 0) ) goto _label
#define ee_bgtz( _a, _label )          if( ((_a)->vx > 0) ) goto _label
#define ee_b( _label )                 goto _label
#define ee_bc1t( _label )              if( pPS2->ee.flags.c ) goto _label
#define ee_bc1f( _label )              if( !pPS2->ee.flags.c ) goto _label

//----------------------------------------------------------------------------
// MISC FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_reset( PS2CTX *pPS2 )
{
   // Set all registers to zero
   memset( &pPS2->ee, 0, sizeof(pPS2->ee) );
}

//----------------------------------------------------------------------------
// DEBUG FUNCTIONS
//----------------------------------------------------------------------------

#ifdef BP_USE_EE_ASSERT

EXTERN_INLINE
void ee_sanity_check( PS2CTX const *pPS2 )
{
   // Make sure constant registers arent trashed
   bp_ee_assert( pPS2->vu0.vf[0].vx == 0.0f );
   bp_ee_assert( pPS2->vu0.vf[0].vy == 0.0f );
   bp_ee_assert( pPS2->vu0.vf[0].vz == 0.0f );
   bp_ee_assert( pPS2->vu0.vf[0].vw == 1.0f );
   bp_ee_assert( pPS2->vu0.vi[0] == 0 );

   // Make sure ee constant registers aren't trashed
   bp_ee_assert( pPS2->ee.gpr[0].vx == 0 );
   bp_ee_assert( pPS2->ee.gpr[0].vy == 0 );
   bp_ee_assert( pPS2->ee.gpr[0].vz == 0 );
   bp_ee_assert( pPS2->ee.gpr[0].vw == 0 );
}

EXTERN_INLINE
void ee_debug_reset( PS2CTX *pPS2 )
{
   eectx_reset( pPS2 );
}

EXTERN_INLINE
void ee_debug_trash( PS2CTX *pPS2 )
{
   int i;

   // Trash float registers
   for( i = 1; i < EE_FPR_REG_COUNT; i++ )
   {
      BP_Float_SetInvalid( &pPS2->ee.fpr[i] );
   }

   // Trash integer registers
   for( i = 1; i < EE_GPR_REG_COUNT; i++ )
   {
      BP_Vec4_SetInvalid( (FVECTOR*)&pPS2->ee.gpr[i] );
   }
}

#else

#define ee_sanity_check(x) (0)
#define ee_debug_reset(x) (0)
#define ee_debug_trash(x) (0)

#endif

//-----------------------------------------------------------------------------
// Transfer functions
//-----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_move_int_gpr(PS2CTX *pPS2, int* dst, const IVECTOR* src)
{
   ee_sanity_check(pPS2);

   *dst = src->vx;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_move_gpr_ptr(PS2CTX *pPS2, IVECTOR* dst, const void* src)
{
   ee_sanity_check(pPS2);

   dst->vx = (int)src;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_move_gpr_int(PS2CTX *pPS2, IVECTOR* dst, const int src)
{
   ee_sanity_check(pPS2);

   dst->vx = src;
}

//-----------------------------------------------------------------------------

// Move word from FPR
EXTERN_INLINE
void eectx_mfc1(PS2CTX *pPS2, IVECTOR* dst, const float src)
{
   ee_sanity_check(pPS2);

   dst->vx = *(int*)&src;
   dst->vy = (dst->vx < 0) ? -1 : 0;
}

//-----------------------------------------------------------------------------

// Move word to FCR
EXTERN_INLINE
void eectx_mtc1(PS2CTX *pPS2, const IVECTOR* src, float* dst)
{
   ee_sanity_check(pPS2);

   *dst = *(float*)&src->vx;
}

//-----------------------------------------------------------------------------

// Quadword move to COP2 (VU0)
EXTERN_INLINE
void eectx_qmtc2(PS2CTX *pPS2, const IVECTOR* src, FVECTOR* dst)
{
   const FVECTOR* src_vec = (const FVECTOR*)src;

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector float *) dst) = *((vector float const *) src_vec );
#else
   dst->vx = src_vec->vx;
   dst->vy = src_vec->vy;
   dst->vz = src_vec->vz;
   dst->vw = src_vec->vw;
#endif
}

//-----------------------------------------------------------------------------

// Quadword move to COP2 (VU0)
EXTERN_INLINE
void eectx_qmtc2_i(PS2CTX *pPS2, const IVECTOR* src, FVECTOR* dst)
{
   const FVECTOR* src_vec = (const FVECTOR*)src;

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector float *) dst) = *((vector float const *) src_vec );
#else
   dst->vx = src_vec->vx;
   dst->vy = src_vec->vy;
   dst->vz = src_vec->vz;
   dst->vw = src_vec->vw;
#endif
}

//-----------------------------------------------------------------------------

// Quadword move to COP2 (VU0)
EXTERN_INLINE
void eectx_qmtc2_ni(PS2CTX *pPS2, const IVECTOR* src, FVECTOR* dst)
{
   const FVECTOR* src_vec = (const FVECTOR*)src;

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector float *) dst) = *((vector float const *) src_vec );
#else
   dst->vx = src_vec->vx;
   dst->vy = src_vec->vy;
   dst->vz = src_vec->vz;
   dst->vw = src_vec->vw;
#endif
}

//-----------------------------------------------------------------------------

// Quadword move to COP2 (VU0) - added float immediate version for ASM
EXTERN_INLINE
void eectx_qmtc2_float_imm(PS2CTX *pPS2, const float src, FVECTOR* dst)
{
   ee_sanity_check(pPS2);

   dst->vx = src;
}

//-----------------------------------------------------------------------------

// Transfer integer data from EE Core to VU
EXTERN_INLINE
void eectx_ctc2_int(PS2CTX *pPS2, const IVECTOR* src, int* dst )
{
   ee_sanity_check(pPS2);

   bp_math_assert( dst == &pPS2->vu0.flags[0].clipping );

   *dst = src->vx;
}

//-----------------------------------------------------------------------------

// Transfer integer data from EE Core to VU
EXTERN_INLINE
void eectx_ctc2_short(PS2CTX *pPS2, const IVECTOR* src, short* dst )
{
   ee_sanity_check(pPS2);

   bp_math_assert( ( dst >= &pPS2->vu0.vi[1] ) && ( dst < &pPS2->vu0.vi[16] ) );

   *dst = src->vx;
}

//-----------------------------------------------------------------------------

// Transfer integer data from EE Core to VU
EXTERN_INLINE
void eectx_ctc2_short_imm(PS2CTX *pPS2, const int imm, short* dst )
{
   ee_sanity_check(pPS2);

   bp_math_assert( ( dst >= &pPS2->vu0.vi[1] ) && ( dst < &pPS2->vu0.vi[16] ) );

   *dst = imm;
}

//-----------------------------------------------------------------------------

// Move control from COP2 (VU0)
EXTERN_INLINE
void eectx_cfc2_int(PS2CTX *pPS2, int* dst, const int* src)
{
   ee_sanity_check(pPS2);

   bp_math_assert( src == &pPS2->vu0.flags[0].clipping );

   *dst = *src;
}

//-----------------------------------------------------------------------------

// Move control from COP2 (VU0)
EXTERN_INLINE
void eectx_cfc2_short(PS2CTX *pPS2, int* dst, const short* src)
{
   ee_sanity_check(pPS2);

   bp_math_assert( ( src >= &pPS2->vu0.vi[1] ) && ( src < &pPS2->vu0.vi[16] ) );

   *dst = *src;
}

//-----------------------------------------------------------------------------

// Store quadword from COP2 (VU0)
EXTERN_INLINE
void eectx_sqc2(PS2CTX *pPS2, const FVECTOR* src, const int offset, void* dst)
{
   FVECTOR* dst_vec = (FVECTOR*)((const char*)dst + offset);

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector float *) dst_vec) = *((vector float const *) src );
#else
   dst_vec->vx = src->vx;
   dst_vec->vy = src->vy;
   dst_vec->vz = src->vz;
   dst_vec->vw = src->vw;
#endif
}

//-----------------------------------------------------------------------------

// Load doubleword
EXTERN_INLINE
void eectx_ld(PS2CTX *pPS2, IVECTOR* dst, const int offset, const void* src)
{
   const int* srci = (const int*)((const char*)src + offset);

   ee_sanity_check(pPS2);

   dst->vx = srci[0];
   dst->vy = srci[1];
}

//-----------------------------------------------------------------------------

// Load immediate
EXTERN_INLINE
void eectx_li(PS2CTX *pPS2, IVECTOR* dst, const int imm)
{
   ee_sanity_check(pPS2);

   dst->vx = imm;
   dst->vy = 0;
}

//-----------------------------------------------------------------------------

// Load quadword
EXTERN_INLINE
void eectx_lq(PS2CTX *pPS2, IVECTOR* dst, const int offset, const void* src)
{
   const IVECTOR* src_vec = (const IVECTOR*)((const char*)src + offset);

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector int *) dst) = *((vector int const *) src_vec );
#else

   dst->vx = src_vec->vx;
   dst->vy = src_vec->vy;
   dst->vz = src_vec->vz;
   dst->vw = src_vec->vw;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_sq(PS2CTX *pPS2, const IVECTOR* src, const int offset, const void* dst)
{
   IVECTOR* dst_vec = (IVECTOR*)((char*)dst + offset);

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector int *) dst_vec) = *((vector int const *) src );
#else
   dst_vec->vx = src->vx;
   dst_vec->vy = src->vy;
   dst_vec->vz = src->vz;
   dst_vec->vw = src->vw;
#endif
}

//-----------------------------------------------------------------------------

// Load quadword to COP2 (VU0)
EXTERN_INLINE
void eectx_lqc2(PS2CTX *pPS2, FVECTOR* dst, const int offset, const void* src)
{
   const FVECTOR* src_vec = (const FVECTOR*)((const char*)src + offset);

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector float *) dst ) = *((vector float const *) src_vec );
#else
   dst->vx = src_vec->vx;
   dst->vy = src_vec->vy;
   dst->vz = src_vec->vz;
   dst->vw = src_vec->vw;
#endif
}

//-----------------------------------------------------------------------------

// Quadword move from COP2 (VU0)
EXTERN_INLINE
void eectx_qmfc2_ni(PS2CTX *pPS2, IVECTOR* dst, const FVECTOR* src)
{
   const int* srci = (const int*)src;

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector int *) dst) = *((vector int const *) srci );
#else
   dst->vx = srci[0];
   dst->vy = srci[1];
   dst->vz = srci[2];
   dst->vw = srci[3];
#endif
}

//-----------------------------------------------------------------------------

// Quadword move from COP2 (VU0)
EXTERN_INLINE
void eectx_qmfc2(PS2CTX *pPS2, IVECTOR* dst, const FVECTOR* src)
{
   const int* srci = (const int*)src;

   ee_sanity_check(pPS2);

#if BP_VITA
   *((vector int *) dst) = *((vector int const *) srci );
#else
   dst->vx = srci[0];
   dst->vy = srci[1];
   dst->vz = srci[2];
   dst->vw = srci[3];
#endif
}

//-----------------------------------------------------------------------------

// Quadword move from COP2 (VU0) - added single int version for asm conversion
EXTERN_INLINE
void eectx_qmfc2_int(PS2CTX *pPS2, int* dst, const FVECTOR* src)
{
   const int* srci = (const int*)src;

   ee_sanity_check(pPS2);

   *dst = srci[0];
}

//-----------------------------------------------------------------------------

// Load word to FPR
EXTERN_INLINE
void eectx_lwc1(PS2CTX *pPS2, float* dst, const float* src)
{
   ee_sanity_check(pPS2);

   bp_math_assert( (dst >= &pPS2->ee.fpr[1]) && (dst <= &pPS2->ee.fpr[16]) );
   *dst = *src;
}

//-----------------------------------------------------------------------------

// Store word from FPR
EXTERN_INLINE
void eectx_swc1(PS2CTX *pPS2,  const float* src, float* dst )
{
   ee_sanity_check(pPS2);

   *dst = *src;
}

//----------------------------------------------------------------------------

// Load upper immediate
EXTERN_INLINE
void eectx_lui(PS2CTX *pPS2,  IVECTOR* rt, const int im )
{
   ee_sanity_check(pPS2);

   bp_math_assert( (rt >= &pPS2->ee.gpr[1]) && (rt <= &pPS2->ee.gpr[16]) );
   rt->vx = im<<16;
   rt->vy = (rt->vx < 0) ? -1 : 0;
}

//----------------------------------------------------------------------------

// 64 bit or immediate
EXTERN_INLINE
void eectx_ori(PS2CTX *pPS2,  IVECTOR* rt, const IVECTOR* rs, const int im )
{
   ee_sanity_check(pPS2);

   bp_math_assert( (rt >= &pPS2->ee.gpr[1]) && (rt <= &pPS2->ee.gpr[16]) );
   rt->vx = rs->vx | im;
   rt->vy = rs->vy;
}

//----------------------------------------------------------------------------

// Load immediate
EXTERN_INLINE
void eectx_li_int(PS2CTX *pPS2,  int* rt, const int im )
{
   ee_sanity_check(pPS2);

   *rt = im;
}

//-----------------------------------------------------------------------------
// Integer functions
//-----------------------------------------------------------------------------

// 32 bit add
EXTERN_INLINE
void eectx_add_int(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const int* rt )
{
   ee_sanity_check(pPS2);

   rd->vx = rs->vx + (*rt);
}

//-----------------------------------------------------------------------------

// 32 bit add
EXTERN_INLINE
void eectx_add(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const void* rt )
{
   ee_sanity_check(pPS2);

   rd->vx = rs->vx + (int)rt;
}

//-----------------------------------------------------------------------------

// 32 bit add
EXTERN_INLINE
void eectx_addi_gpr(PS2CTX *pPS2,  IVECTOR* rd, const int imm )
{
   ee_sanity_check(pPS2);

   rd->vx += imm;
}

//-----------------------------------------------------------------------------

// 32 bit add
EXTERN_INLINE
void eectx_addi_ptr(PS2CTX *pPS2,  void** p, const int imm )
{
   char** dst = (char**)p;
   
   ee_sanity_check(pPS2);

   *dst += imm;
}

//-----------------------------------------------------------------------------

// 32 bit add
EXTERN_INLINE
void eectx_addi_gpr_gpr(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const int imm )
{
   ee_sanity_check(pPS2);

   rd->vx = rs->vx + imm;
}

//-----------------------------------------------------------------------------

// unsigned 32 bit add
EXTERN_INLINE
void eectx_addiu_gpr_gpr(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const unsigned int imm )
{
   ee_sanity_check(pPS2);

   rd->vx = (int)((const unsigned int)rs->vx + imm);
}

//-----------------------------------------------------------------------------

// unsigned 32 bit add
EXTERN_INLINE
void eectx_addiu_gpr_ptr(PS2CTX *pPS2,  IVECTOR* rd, const void* p, const unsigned int imm )
{
   ee_sanity_check(pPS2);

   rd->vx = (int)((const unsigned int)p + imm);
}

//-----------------------------------------------------------------------------

// 64 bit and
EXTERN_INLINE
void eectx_and(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   ee_sanity_check(pPS2);

   rd->vx = rs->vx & rt->vx;
   rd->vy = rs->vy & rt->vy;
}

//-----------------------------------------------------------------------------

// 64 bit and immediate
EXTERN_INLINE
void eectx_andi(PS2CTX *pPS2,  IVECTOR* rt, const IVECTOR* rs, const int imm )
{
   ee_sanity_check(pPS2);

   rt->vx = rs->vx & imm;
   rt->vy = 0;
}

//-----------------------------------------------------------------------------

// 32 bit and
EXTERN_INLINE
void eectx_and_int(PS2CTX *pPS2,  int* rd, const int* rs, const IVECTOR* rt )
{
   ee_sanity_check(pPS2);

   *rd = (*rd) & (rt->vx);
}

//-----------------------------------------------------------------------------

// 32 bit and immediate
EXTERN_INLINE
void eectx_andi_int(PS2CTX *pPS2,  int* rt, const int* rs, const int imm )
{
   ee_sanity_check(pPS2);

   *rt = (*rs) & imm;
}

//-----------------------------------------------------------------------------

// 64 bit Doubleword shift left logical variable
EXTERN_INLINE
void eectx_dsllv(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   const unsigned int x = rs->vx;
   const unsigned int y = rs->vy;
   const int shifts = rt->vx;

   ee_sanity_check(pPS2);

   bp_math_assert( shifts >= 0 );

   rd->vx = x << shifts;
   rd->vy = y << shifts;
   rd->vy |= (x >> (32-shifts));
}

//-----------------------------------------------------------------------------

// 64 bit Doubleword shift right logical variable
EXTERN_INLINE
void eectx_dsrlv(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   const unsigned int x = rs->vx;
   const unsigned int y = rs->vy;
   const int shifts = rt->vx;

   ee_sanity_check(pPS2);

   bp_math_assert( shifts >= 0 );

   rd->vy = y >> shifts;
   rd->vx = x >> shifts;
   rd->vx |= (y << (32-shifts));
}

//-----------------------------------------------------------------------------

// 64 bit Doubleword shift right logical
EXTERN_INLINE
void eectx_dsrl(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const int shifts )
{
   const unsigned int x = rs->vx;
   const unsigned int y = rs->vy;

   ee_sanity_check(pPS2);

   rd->vy = y >> shifts;
   rd->vx = x >> shifts;
   rd->vx |= (y << (32-shifts));
}

//-----------------------------------------------------------------------------

// 32 bit shift word left logical (sign extended to 64 bits)
EXTERN_INLINE
void eectx_sll(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rt, const int sa )
{
   ee_sanity_check(pPS2);

   rd->vx = rt->vx << sa;
   rd->vy = ( rd->vx < 0 ) ? -1 : 0;
}

//-----------------------------------------------------------------------------

#if BP_VITA

#define eectx_psllw(pPS2,rd,rt,sa) (*((vector int *) (rd)) = vshlq_n_s32( *((vector int const *) (rt)), (sa) ))

#else

// parallel shift left logical word
EXTERN_INLINE
void eectx_psllw(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rt, const int sa )
{
   ee_sanity_check(pPS2);
   rd->vx = rt->vx << sa;
   rd->vy = rt->vy << sa;
   rd->vz = rt->vz << sa;
   rd->vw = rt->vw << sa;
}

#endif

//-----------------------------------------------------------------------------

// Parallel Copy Lower Doubleword
EXTERN_INLINE
void eectx_pcpyld(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   const int A0 = rs->vx;
   const int A1 = rs->vy;
   const int B0 = rt->vx;
   const int B1 = rt->vy;

   ee_sanity_check(pPS2);

   rd->vx = B0;
   rd->vy = B1;
   rd->vz = A0;
   rd->vw = A1;
}

//-----------------------------------------------------------------------------

// Parallel Copy Upper Doubleword
EXTERN_INLINE
void eectx_pcpyud(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   const int A0 = rs->vz;
   const int A1 = rs->vw;
   const int B0 = rt->vz;
   const int B1 = rt->vw;

   ee_sanity_check(pPS2);

   rd->vx = A0;
   rd->vy = A1;
   rd->vz = B0;
   rd->vw = B1;
}

//-----------------------------------------------------------------------------

// Parallel shift right arithmetic word
EXTERN_INLINE
void eectx_psraw(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rt, const int sa )
{
   ee_sanity_check(pPS2);

   rd->vx = rt->vx >> sa;
   rd->vy = rt->vy >> sa;
   rd->vz = rt->vz >> sa;
   rd->vw = rt->vw >> sa;
}

//-----------------------------------------------------------------------------

// Parallel Extend Lower from Halfword
EXTERN_INLINE
void eectx_pextlh(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
#if BPE_IS_ENDIAN_BIG()
   // PS3, Xbox360
   const int A0 = (rs->vx >> 16) & 0xffff;
   const int A1 = (rs->vx >>  0) & 0xffff;
   const int A2 = (rs->vy >> 16) & 0xffff;
   const int A3 = (rs->vy >>  0) & 0xffff;

   const int B0 = (rt->vx >> 16) & 0xffff;
   const int B1 = (rt->vx >>  0) & 0xffff;
   const int B2 = (rt->vy >> 16) & 0xffff;
   const int B3 = (rt->vy >>  0) & 0xffff;
#else
   // PC
   const int A0 = (rs->vx >>  0) & 0xffff;
   const int A1 = (rs->vx >> 16) & 0xffff;
   const int A2 = (rs->vy >>  0) & 0xffff;
   const int A3 = (rs->vy >> 16) & 0xffff;

   const int B0 = (rt->vx >>  0) & 0xffff;
   const int B1 = (rt->vx >> 16) & 0xffff;
   const int B2 = (rt->vy >>  0) & 0xffff;
   const int B3 = (rt->vy >> 16) & 0xffff;
#endif

   ee_sanity_check(pPS2);

   rd->vx = B0 | (A0 << 16);
   rd->vy = B1 | (A1 << 16);
   rd->vz = B2 | (A2 << 16);
   rd->vw = B3 | (A3 << 16);
}

//-----------------------------------------------------------------------------

// Parallel Extend Lower from word
EXTERN_INLINE
void eectx_pextlw(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   const int A0 = rs->vx;
   const int A1 = rs->vy;
   const int B0 = rt->vx;
   const int B1 = rt->vy;

   ee_sanity_check(pPS2);

   rd->vx = B0;
   rd->vy = A0;
   rd->vz = B1;
   rd->vw = A1;
}
//-----------------------------------------------------------------------------

// Parallel Extend Upper from word
EXTERN_INLINE
void eectx_pextuw(PS2CTX *pPS2,  IVECTOR* rd, const IVECTOR* rs, const IVECTOR* rt )
{
   const int A0 = rs->vz;
   const int A1 = rs->vw;
   const int B0 = rt->vz;
   const int B1 = rt->vw;

   ee_sanity_check(pPS2);

   rd->vx = B0;
   rd->vy = A0;
   rd->vz = B1;
   rd->vw = A1;
}

//-----------------------------------------------------------------------------
// Single precision float functions
//-----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_sqrt_s(PS2CTX *pPS2,  float* fd, const float* ft )
{
   ee_sanity_check(pPS2);

   *fd = BP_EE_Sqrt( *ft );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_div_s(PS2CTX *pPS2,  float* fd, const float* fs, const float* ft )
{
   ee_sanity_check(pPS2);

   *fd = BP_SafeDivideF( *fs, *ft );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_abs_s(PS2CTX *pPS2,  float* fd, const float* fs )
{
   ee_sanity_check(pPS2);

   *fd = BP_Fabsf( *fs );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_c_le_s(PS2CTX *pPS2,  const float* fs, const float* ft )
{
   ee_sanity_check(pPS2);

   pPS2->ee.flags.c = (*fs <= *ft);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_c_eq_s(PS2CTX *pPS2,  const float* fs, const float* ft )
{
   ee_sanity_check(pPS2);

   pPS2->ee.flags.c = (*fs == *ft);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void eectx_c_lt_s(PS2CTX *pPS2,  const float* fs, const float* ft )
{
   ee_sanity_check(pPS2);

   pPS2->ee.flags.c = (*fs < *ft);
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#define ee_move_int_gpr(...) eectx_move_int_gpr(pPS2,__VA_ARGS__) 
#define ee_move_gpr_ptr(...) eectx_move_gpr_ptr(pPS2,__VA_ARGS__) 
#define ee_move_gpr_int(...) eectx_move_gpr_int(pPS2,__VA_ARGS__) 
#define ee_mfc1(...) eectx_mfc1(pPS2,__VA_ARGS__) 
#define ee_mtc1(...) eectx_mtc1(pPS2,__VA_ARGS__) 
#define ee_qmtc2(...) eectx_qmtc2(pPS2,__VA_ARGS__) 
#define ee_qmtc2_i(...) eectx_qmtc2_i(pPS2,__VA_ARGS__) 
#define ee_qmtc2_ni(...) eectx_qmtc2_ni(pPS2,__VA_ARGS__) 
#define ee_qmtc2_float_imm(...) eectx_qmtc2_float_imm(pPS2,__VA_ARGS__) 
#define ee_ctc2_int(...) eectx_ctc2_int(pPS2,__VA_ARGS__) 
#define ee_ctc2_short(...) eectx_ctc2_short(pPS2,__VA_ARGS__) 
#define ee_ctc2_short_imm(...) eectx_ctc2_short_imm(pPS2,__VA_ARGS__) 
#define ee_cfc2_int(...) eectx_cfc2_int(pPS2,__VA_ARGS__) 
#define ee_cfc2_short(...) eectx_cfc2_short(pPS2,__VA_ARGS__) 
#define ee_sqc2(...) eectx_sqc2(pPS2,__VA_ARGS__) 
#define ee_ld(...) eectx_ld(pPS2,__VA_ARGS__) 
#define ee_li(...) eectx_li(pPS2,__VA_ARGS__) 
#define ee_lq(...) eectx_lq(pPS2,__VA_ARGS__) 
#define ee_sq(...) eectx_sq(pPS2,__VA_ARGS__) 
#define ee_lqc2(...) eectx_lqc2(pPS2,__VA_ARGS__) 
#define ee_qmfc2_ni(...) eectx_qmfc2_ni(pPS2,__VA_ARGS__) 
#define ee_qmfc2(...) eectx_qmfc2(pPS2,__VA_ARGS__) 
#define ee_qmfc2_int(...) eectx_qmfc2_int(pPS2,__VA_ARGS__) 
#define ee_lwc1(...) eectx_lwc1(pPS2,__VA_ARGS__) 
#define ee_swc1(...) eectx_swc1(pPS2,__VA_ARGS__) 
#define ee_lui(...) eectx_lui(pPS2,__VA_ARGS__) 
#define ee_ori(...) eectx_ori(pPS2,__VA_ARGS__) 
#define ee_li_int(...) eectx_li_int(pPS2,__VA_ARGS__) 
#define ee_add_int(...) eectx_add_int(pPS2,__VA_ARGS__) 
#define ee_add(...) eectx_add(pPS2,__VA_ARGS__) 
#define ee_addi_gpr(...) eectx_addi_gpr(pPS2,__VA_ARGS__) 
#define ee_addi_ptr(...) eectx_addi_ptr(pPS2,__VA_ARGS__) 
#define ee_addi_gpr_gpr(...) eectx_addi_gpr_gpr(pPS2,__VA_ARGS__) 
#define ee_addiu_gpr_gpr(...) eectx_addiu_gpr_gpr(pPS2,__VA_ARGS__) 
#define ee_addiu_gpr_ptr(...) eectx_addiu_gpr_ptr(pPS2,__VA_ARGS__) 
#define ee_and(...) eectx_and(pPS2,__VA_ARGS__) 
#define ee_andi(...) eectx_andi(pPS2,__VA_ARGS__) 
#define ee_and_int(...) eectx_and_int(pPS2,__VA_ARGS__) 
#define ee_andi_int(...) eectx_andi_int(pPS2,__VA_ARGS__) 
#define ee_dsllv(...) eectx_dsllv(pPS2,__VA_ARGS__) 
#define ee_dsrlv(...) eectx_dsrlv(pPS2,__VA_ARGS__) 
#define ee_dsrl(...) eectx_dsrl(pPS2,__VA_ARGS__) 
#define ee_sll(...) eectx_sll(pPS2,__VA_ARGS__) 
#define ee_psllw(...) eectx_psllw(pPS2,__VA_ARGS__) 
#define ee_psllw(...) eectx_psllw(pPS2,__VA_ARGS__) 
#define ee_pcpyld(...) eectx_pcpyld(pPS2,__VA_ARGS__) 
#define ee_pcpyud(...) eectx_pcpyud(pPS2,__VA_ARGS__) 
#define ee_psraw(...) eectx_psraw(pPS2,__VA_ARGS__) 
#define ee_pextlh(...) eectx_pextlh(pPS2,__VA_ARGS__) 
#define ee_pextlw(...) eectx_pextlw(pPS2,__VA_ARGS__) 
#define ee_pextuw(...) eectx_pextuw(pPS2,__VA_ARGS__) 
#define ee_sqrt_s(...) eectx_sqrt_s(pPS2,__VA_ARGS__) 
#define ee_div_s(...) eectx_div_s(pPS2,__VA_ARGS__) 
#define ee_abs_s(...) eectx_abs_s(pPS2,__VA_ARGS__) 
#define ee_c_le_s(...) eectx_c_le_s(pPS2,__VA_ARGS__) 
#define ee_c_eq_s(...) eectx_c_eq_s(pPS2,__VA_ARGS__) 
#define ee_c_lt_s(...) eectx_c_lt_s(pPS2,__VA_ARGS__) 

#define ee_reset() eectx_reset(pPS2)

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_EE_H__
