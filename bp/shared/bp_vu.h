//-----------------------------------------------------------------------------
// vu_u.h
//-----------------------------------------------------------------------------
#ifndef __BP_VU_H__
#define __BP_VU_H__

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

// Enable/disable vu math asserts (very slow, but catches invalid floats!)
#if defined(_DEBUG) && defined(STEVEB)
#define BP_USE_VU_ASSERT
#endif

// Declare vu break/assert
#ifdef BP_USE_VU_ASSERT
#define bp_vu_break               BP_BREAK
#define bp_vu_assert(__exp__)     if(!(__exp__)) bp_vu_break
#else
#define bp_vu_break
#define bp_vu_assert(__exp__)
#endif


//----------------------------------------------------------------------------
// VU flags
//----------------------------------------------------------------------------

#define VU_STATUS_FLAG_Z      (1<<0)
#define VU_STATUS_FLAG_S      (1<<1)
#define VU_STATUS_FLAG_U      (1<<2)
#define VU_STATUS_FLAG_O      (1<<3)
#define VU_STATUS_FLAG_I      (1<<4)
#define VU_STATUS_FLAG_D      (1<<5)

#define VU_STATUS_FLAG_ZS     (1<<6)
#define VU_STATUS_FLAG_SS     (1<<7)
#define VU_STATUS_FLAG_US     (1<<8)
#define VU_STATUS_FLAG_OS     (1<<9)
#define VU_STATUS_FLAG_IS     (1<<10)
#define VU_STATUS_FLAG_DS     (1<<11)

#define VU_MAC_FLAG_Zw        (1<<0)
#define VU_MAC_FLAG_Zz        (1<<1)
#define VU_MAC_FLAG_Zy        (1<<2)
#define VU_MAC_FLAG_Zx        (1<<3)

#define VU_MAC_FLAG_Sw        (1<<4)
#define VU_MAC_FLAG_Sz        (1<<5)
#define VU_MAC_FLAG_Sy        (1<<6)
#define VU_MAC_FLAG_Sx        (1<<7)

#define VU_MAC_FLAG_Uw        (1<<8)
#define VU_MAC_FLAG_Uz        (1<<9)
#define VU_MAC_FLAG_Uy        (1<<10)
#define VU_MAC_FLAG_Ux        (1<<11)

#define VU_MAC_FLAG_Ow        (1<<12)
#define VU_MAC_FLAG_Oz        (1<<13)
#define VU_MAC_FLAG_Oy        (1<<14)
#define VU_MAC_FLAG_Ox        (1<<15)

#define VU_STATUS_FLAGS       (VU_STATUS_FLAG_Z | VU_STATUS_FLAG_S | VU_STATUS_FLAG_U | VU_STATUS_FLAG_O | VU_STATUS_FLAG_I | VU_STATUS_FLAG_D)
#define VU_STATUS_FLAGS_S     (VU_STATUS_FLAG_ZS | VU_STATUS_FLAG_SS | VU_STATUS_FLAG_US | VU_STATUS_FLAG_OS | VU_STATUS_FLAG_IS | VU_STATUS_FLAG_DS)

//-----------------------------------------------------------------------------
// Register defines
//-----------------------------------------------------------------------------

#if MGS_VERSION == 3

#ifndef ACC
#define ACC    &pPS2->vu0.acc
#endif

#ifndef Q
#define Q      &pPS2->vu0.q
#endif

#ifndef I
#define I      &pPS2->vu0.i
#endif

#define vf00         (&pPS2->vu0.vf[0])
#define vf01         (&pPS2->vu0.vf[1])
#define vf02         (&pPS2->vu0.vf[2])
#define vf03         (&pPS2->vu0.vf[3])
#define vf04         (&pPS2->vu0.vf[4])
#define vf05         (&pPS2->vu0.vf[5])
#define vf06         (&pPS2->vu0.vf[6])
#define vf07         (&pPS2->vu0.vf[7])
#define vf08         (&pPS2->vu0.vf[8])
#define vf09         (&pPS2->vu0.vf[9])
#define vf10         (&pPS2->vu0.vf[10])
#define vf11         (&pPS2->vu0.vf[11])
#define vf12         (&pPS2->vu0.vf[12])
#define vf13         (&pPS2->vu0.vf[13])
#define vf14         (&pPS2->vu0.vf[14])
#define vf15         (&pPS2->vu0.vf[15])
#define vf16         (&pPS2->vu0.vf[16])
#define vf17         (&pPS2->vu0.vf[17])
#define vf18         (&pPS2->vu0.vf[18])
#define vf19         (&pPS2->vu0.vf[19])
#define vf20         (&pPS2->vu0.vf[20])
#define vf21         (&pPS2->vu0.vf[21])
#define vf22         (&pPS2->vu0.vf[22])
#define vf23         (&pPS2->vu0.vf[23])
#define vf24         (&pPS2->vu0.vf[24])
#define vf25         (&pPS2->vu0.vf[25])
#define vf26         (&pPS2->vu0.vf[26])
#define vf27         (&pPS2->vu0.vf[27])
#define vf28         (&pPS2->vu0.vf[28])
#define vf29         (&pPS2->vu0.vf[29])
#define vf30         (&pPS2->vu0.vf[30])
#define vf31         (&pPS2->vu0.vf[31])

#define vf00w        (&pPS2->vu0.vf[0].vw)
#define vf01w        (&pPS2->vu0.vf[1].vw)
#define vf02w        (&pPS2->vu0.vf[2].vw)
#define vf03w        (&pPS2->vu0.vf[3].vw)
#define vf04w        (&pPS2->vu0.vf[4].vw)
#define vf05w        (&pPS2->vu0.vf[5].vw)
#define vf06w        (&pPS2->vu0.vf[6].vw)
#define vf07w        (&pPS2->vu0.vf[7].vw)
#define vf08w        (&pPS2->vu0.vf[8].vw)
#define vf09w        (&pPS2->vu0.vf[9].vw)
#define vf10w        (&pPS2->vu0.vf[10].vw)
#define vf11w        (&pPS2->vu0.vf[11].vw)
#define vf12w        (&pPS2->vu0.vf[12].vw)
#define vf13w        (&pPS2->vu0.vf[13].vw)
#define vf19w        (&pPS2->vu0.vf[19].vw)
#define vf20w        (&pPS2->vu0.vf[20].vw)
#define vf22w        (&pPS2->vu0.vf[22].vw)
#define vf25w        (&pPS2->vu0.vf[25].vw)
#define vf26w        (&pPS2->vu0.vf[26].vw)
#define vf29w        (&pPS2->vu0.vf[29].vw)

#define vi00         (&pPS2->vu0.vi[0])
#define vi01         (&pPS2->vu0.vi[1])
#define vi02         (&pPS2->vu0.vi[2])
#define vi03         (&pPS2->vu0.vi[3])
#define vi04         (&pPS2->vu0.vi[4])
#define vi05         (&pPS2->vu0.vi[5])
#define vi06         (&pPS2->vu0.vi[6])
#define vi07         (&pPS2->vu0.vi[7])
#define vi08         (&pPS2->vu0.vi[8])
#define vi09         (&pPS2->vu0.vi[9])
#define vi10         (&pPS2->vu0.vi[10])
#define vi11         (&pPS2->vu0.vi[11])
#define vi12         (&pPS2->vu0.vi[12])
#define vi13         (&pPS2->vu0.vi[13])
#define vi14         (&pPS2->vu0.vi[14])
#define vi15         (&pPS2->vu0.vi[15])
#define vi18         (&pPS2->vu0.flags[0].clipping)

#define vu_flags0    (&pPS2->vu0.flags[0])
#define vu_flags1    (&pPS2->vu0.flags[1])
#define vu_flags2    (&pPS2->vu0.flags[2])
#define vu_flags3    (&pPS2->vu0.flags[3])
#define vu_flags4    (&pPS2->vu0.flags[4])
#define vu_flags5    (&pPS2->vu0.flags[5])
#define vu_flags6    (&pPS2->vu0.flags[6])
#define vu_flags7    (&pPS2->vu0.flags[7])

#endif// #if MGS_VERSION == 3


//-----------------------------------------------------------------------------
// Branching macros
//-----------------------------------------------------------------------------

#define vu_b_ibne( _it, _is, _label ) if( (*(_it)) != (*(_is)) ) goto _label
#define vu_b_ibeq( _it, _is, _label ) if( (*(_it)) == (*(_is)) ) goto _label
#define vu_b_ibgtz( _is, _label )     if( (*(_is)) > 0 ) goto _label
#define vu_b_ibgez( _is, _label )     if( (*(_is)) >= 0 ) goto _label
#define vu_b_b( _label )              goto _label
#define vu_b_bal( _it, _label )       _label(pPS2)

//----------------------------------------------------------------------------
// MISC FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_reset( PS2CTX *pPS2 )
{
   // Set all registers and memory to zero
   memset( &pPS2->vu0, 0, sizeof(pPS2->vu0) );

   // vf00 is always constant {0,0,0,1}
   BP_Vec4_Set( &pPS2->vu0.vf[0], 0.0f, 0.0f, 0.0f, 1.0f );
}

//----------------------------------------------------------------------------
// DEBUG FUNCTIONS
//----------------------------------------------------------------------------

#ifdef BP_USE_VU_ASSERT

EXTERN_INLINE
void vu_sanity_check( PS2CTX *pPS2 )
{
   // Make sure constant registers arent trashed
   bp_vu_assert( pPS2->vu0.vf[0].vx == 0.0f );
   bp_vu_assert( pPS2->vu0.vf[0].vy == 0.0f );
   bp_vu_assert( pPS2->vu0.vf[0].vz == 0.0f );
   bp_vu_assert( pPS2->vu0.vf[0].vw == 1.0f );
   bp_vu_assert( pPS2->vu0.vi[0] == 0 );

   // Make sure ee constant registers aren't trashed
   bp_vu_assert( pPS2->ee.gpr[0].vx == 0 );
   bp_vu_assert( pPS2->ee.gpr[0].vy == 0 );
   bp_vu_assert( pPS2->ee.gpr[0].vz == 0 );
   bp_vu_assert( pPS2->ee.gpr[0].vw == 0 );
}

EXTERN_INLINE
void vu_debug_reset( PS2CTX *pPS2 )
{
   vuctx_reset( pPS2 );
}

EXTERN_INLINE
void vu_debug_trash( PS2CTX *pPS2 )
{
   int i;

   // Trash misc registers
   BP_Float_SetInvalid( &pPS2->vu0.i );
   BP_Float_SetInvalid( &pPS2->vu0.q );
   BP_Vec4_SetInvalid( &pPS2->vu0.acc );

   // Trash float registers
   for( i = 1; i < VU_FLT_REG_COUNT; i++ )
   {
      BP_Vec4_SetInvalid( &pPS2->vu0.vf[i] );
   }

   // Trash integer registers
   for( i = 1; i < VU_INT_REG_COUNT; i++ )
   {
      pPS2->vu0.vi[i] = 0x3333;
   }

   // Trash memory
   for( i = 0; i < VU_MEM_COUNT; i++ )
   {
      BP_Vec4_SetInvalid( &pPS2->vu0.memory[i].fv );
   }
}
#else

#define vu_sanity_check(pPS2)
#define vu_debug_trash(pPS2)
#define vu_debug_reset(pPS2)

#endif

//----------------------------------------------------------------------------

// defines

#define vu_reset(...) vuctx_reset(pPS2,__VA_ARGS__) 
#define vu_callms(...) vuctx_callms(pPS2,__VA_ARGS__) 
#define vu_update_status_flags(...) vuctx_update_status_flags(pPS2,__VA_ARGS__) 
#define vu_update_status_flags_xyz(...) vuctx_update_status_flags_xyz(pPS2,__VA_ARGS__) 
#define vu_update_status_flags_x(...) vuctx_update_status_flags_x(pPS2,__VA_ARGS__) 
#define vu_update_status_flags_w(...) vuctx_update_status_flags_w(pPS2,__VA_ARGS__) 
#define vu_fsset(...) vuctx_fsset(pPS2,__VA_ARGS__) 
#define vu_fsand(...) vuctx_fsand(pPS2,__VA_ARGS__) 
#define vu_fmand(...) vuctx_fmand(pPS2,__VA_ARGS__) 
#define vu_fcset(...) vuctx_fcset(pPS2,__VA_ARGS__) 
#define vu_fcget(...) vuctx_fcget(pPS2,__VA_ARGS__) 
#define vu_iadd(...) vuctx_iadd(pPS2,__VA_ARGS__) 
#define vu_iaddi(...) vuctx_iaddi(pPS2,__VA_ARGS__) 
#define vu_iaddiu(...) vuctx_iaddiu(pPS2,__VA_ARGS__) 
#define vu_isubiu(...) vuctx_isubiu(pPS2,__VA_ARGS__) 
#define vu_isub(...) vuctx_isub(pPS2,__VA_ARGS__) 
#define vu_iand(...) vuctx_iand(pPS2,__VA_ARGS__) 
#define vu_ior(...) vuctx_ior(pPS2,__VA_ARGS__) 
#define vu_loi(...) vuctx_loi(pPS2,__VA_ARGS__) 
#define vu_move(...) vuctx_move(pPS2,__VA_ARGS__) 
#define vu_move_xyzw(...) vuctx_move_xyzw(pPS2,__VA_ARGS__) 
#define vu_move_xyz(...) vuctx_move_xyz(pPS2,__VA_ARGS__) 
#define vu_move_xy(...) vuctx_move_xy(pPS2,__VA_ARGS__) 
#define vu_move_xz(...) vuctx_move_xz(pPS2,__VA_ARGS__) 
#define vu_move_yz(...) vuctx_move_yz(pPS2,__VA_ARGS__) 
#define vu_move_x(...) vuctx_move_x(pPS2,__VA_ARGS__) 
#define vu_move_y(...) vuctx_move_y(pPS2,__VA_ARGS__) 
#define vu_move_z(...) vuctx_move_z(pPS2,__VA_ARGS__) 
#define vu_move_w(...) vuctx_move_w(pPS2,__VA_ARGS__) 
#define vu_mul(...) vuctx_mul(pPS2,__VA_ARGS__) 
#define vu_mulx_xyz(...) vuctx_mulx_xyz(pPS2,__VA_ARGS__) 
#define vu_mulx_x(...) vuctx_mulx_x(pPS2,__VA_ARGS__) 
#define vu_mulx_w(...) vuctx_mulx_w(pPS2,__VA_ARGS__) 
#define vu_muly_w(...) vuctx_muly_w(pPS2,__VA_ARGS__) 
#define vu_mul_xyz(...) vuctx_mul_xyz(pPS2,__VA_ARGS__) 
#define vu_mul_xy(...) vuctx_mul_xy(pPS2,__VA_ARGS__) 
#define vu_mul_w(...) vuctx_mul_w(pPS2,__VA_ARGS__) 
#define vu_mul_xyz_sf(...) vuctx_mul_xyz_sf(pPS2,__VA_ARGS__) 
#define vu_mul_x(...) vuctx_mul_x(pPS2,__VA_ARGS__) 
#define vu_mulx(...) vuctx_mulx(pPS2,__VA_ARGS__) 
#define vu_muly(...) vuctx_muly(pPS2,__VA_ARGS__) 
#define vu_mulw(...) vuctx_mulw(pPS2,__VA_ARGS__) 
#define vu_mulz_w(...) vuctx_mulz_w(pPS2,__VA_ARGS__) 
#define vu_mulw_x(...) vuctx_mulw_x(pPS2,__VA_ARGS__) 
#define vu_mulw_xyz(...) vuctx_mulw_xyz(pPS2,__VA_ARGS__) 
#define vu_mulw_w(...) vuctx_mulw_w(pPS2,__VA_ARGS__) 
#define vu_adda_xyz(...) vuctx_adda_xyz(pPS2,__VA_ARGS__) 
#define vu_suba_xyz(...) vuctx_suba_xyz(pPS2,__VA_ARGS__) 
#define vu_addax_xyz(...) vuctx_addax_xyz(pPS2,__VA_ARGS__) 
#define vu_addaw_xyz(...) vuctx_addaw_xyz(pPS2,__VA_ARGS__) 
#define vu_mula_w(...) vuctx_mula_w(pPS2,__VA_ARGS__) 
#define vu_mulax(...) vuctx_mulax(pPS2,__VA_ARGS__) 
#define vu_mulaw(...) vuctx_mulaw(pPS2,__VA_ARGS__) 
#define vu_mulaw_w(...) vuctx_mulaw_w(pPS2,__VA_ARGS__) 
#define vu_mula_xy(...) vuctx_mula_xy(pPS2,__VA_ARGS__) 
#define vu_mulaw_xy(...) vuctx_mulaw_xy(pPS2,__VA_ARGS__) 
#define vu_mulai_xyz(...) vuctx_mulai_xyz(pPS2,__VA_ARGS__) 
#define vu_mulai_w(...) vuctx_mulai_w(pPS2,__VA_ARGS__) 
#define vu_mula(...) vuctx_mula(pPS2,__VA_ARGS__) 
#define vu_mulax_xyz(...) vuctx_mulax_xyz(pPS2,__VA_ARGS__) 
#define vu_mulax_yz(...) vuctx_mulax_yz(pPS2,__VA_ARGS__) 
#define vu_mulax_x(...) vuctx_mulax_x(pPS2,__VA_ARGS__) 
#define vu_mulax_y(...) vuctx_mulax_y(pPS2,__VA_ARGS__) 
#define vu_mulax_z(...) vuctx_mulax_z(pPS2,__VA_ARGS__) 
#define vu_mulax_w(...) vuctx_mulax_w(pPS2,__VA_ARGS__) 
#define vu_mulax_w_sf(...) vuctx_mulax_w_sf(pPS2,__VA_ARGS__) 
#define vu_mulaq_w(...) vuctx_mulaq_w(pPS2,__VA_ARGS__) 
#define vu_mulay_w(...) vuctx_mulay_w(pPS2,__VA_ARGS__) 
#define vu_mulay_xz(...) vuctx_mulay_xz(pPS2,__VA_ARGS__) 
#define vu_mulaz_xy(...) vuctx_mulaz_xy(pPS2,__VA_ARGS__) 
#define vu_mulaw_xyz(...) vuctx_mulaw_xyz(pPS2,__VA_ARGS__) 
#define vu_maddx_z(...) vuctx_maddx_z(pPS2,__VA_ARGS__) 
#define vu_maddx(...) vuctx_maddx(pPS2,__VA_ARGS__) 
#define vu_maddaw_xy(...) vuctx_maddaw_xy(pPS2,__VA_ARGS__) 
#define vu_maddaw_xyz(...) vuctx_maddaw_xyz(pPS2,__VA_ARGS__) 
#define vu_madday(...) vuctx_madday(pPS2,__VA_ARGS__) 
#define vu_maddaz(...) vuctx_maddaz(pPS2,__VA_ARGS__) 
#define vu_maddax_w(...) vuctx_maddax_w(pPS2,__VA_ARGS__) 
#define vu_madday_w(...) vuctx_madday_w(pPS2,__VA_ARGS__) 
#define vu_maddaz_w(...) vuctx_maddaz_w(pPS2,__VA_ARGS__) 
#define vu_madday_xyz(...) vuctx_madday_xyz(pPS2,__VA_ARGS__) 
#define vu_madday_x(...) vuctx_madday_x(pPS2,__VA_ARGS__) 
#define vu_madday_y(...) vuctx_madday_y(pPS2,__VA_ARGS__) 
#define vu_madday_z(...) vuctx_madday_z(pPS2,__VA_ARGS__) 
#define vu_msubax_w(...) vuctx_msubax_w(pPS2,__VA_ARGS__) 
#define vu_msubax_y(...) vuctx_msubax_y(pPS2,__VA_ARGS__) 
#define vu_msubax_z(...) vuctx_msubax_z(pPS2,__VA_ARGS__) 
#define vu_msubax(...) vuctx_msubax(pPS2,__VA_ARGS__) 
#define vu_msubay(...) vuctx_msubay(pPS2,__VA_ARGS__) 
#define vu_msubay_x(...) vuctx_msubay_x(pPS2,__VA_ARGS__) 
#define vu_msuba_w(...) vuctx_msuba_w(pPS2,__VA_ARGS__) 
#define vu_msubay_w(...) vuctx_msubay_w(pPS2,__VA_ARGS__) 
#define vu_msubaz_w(...) vuctx_msubaz_w(pPS2,__VA_ARGS__) 
#define vu_maddai_w(...) vuctx_maddai_w(pPS2,__VA_ARGS__) 
#define vu_maddai_w_sf(...) vuctx_maddai_w_sf(pPS2,__VA_ARGS__) 
#define vu_maddi_xyz(...) vuctx_maddi_xyz(pPS2,__VA_ARGS__) 
#define vu_maddi_w(...) vuctx_maddi_w(pPS2,__VA_ARGS__) 
#define vu_madd_xyz(...) vuctx_madd_xyz(pPS2,__VA_ARGS__) 
#define vu_maddw_xy(...) vuctx_maddw_xy(pPS2,__VA_ARGS__) 
#define vu_maddw_w(...) vuctx_maddw_w(pPS2,__VA_ARGS__) 
#define vu_maddaz_xyz(...) vuctx_maddaz_xyz(pPS2,__VA_ARGS__) 
#define vu_maddy_w(...) vuctx_maddy_w(pPS2,__VA_ARGS__) 
#define vu_maddy_x(...) vuctx_maddy_x(pPS2,__VA_ARGS__) 
#define vu_maddz(...) vuctx_maddz(pPS2,__VA_ARGS__) 
#define vu_maddw(...) vuctx_maddw(pPS2,__VA_ARGS__) 
#define vu_maddw_xyz(...) vuctx_maddw_xyz(pPS2,__VA_ARGS__) 
#define vu_maddw_xyz_sf(...) vuctx_maddw_xyz_sf(pPS2,__VA_ARGS__) 
#define vu_maddz_xyz(...) vuctx_maddz_xyz(pPS2,__VA_ARGS__) 
#define vu_maddz_x(...) vuctx_maddz_x(pPS2,__VA_ARGS__) 
#define vu_maddz_y(...) vuctx_maddz_y(pPS2,__VA_ARGS__) 
#define vu_maddz_z(...) vuctx_maddz_z(pPS2,__VA_ARGS__) 
#define vu_maddz_w(...) vuctx_maddz_w(pPS2,__VA_ARGS__) 
#define vu_maddz_w_sf(...) vuctx_maddz_w_sf(pPS2,__VA_ARGS__) 
#define vu_maddq_xyz(...) vuctx_maddq_xyz(pPS2,__VA_ARGS__) 
#define vu_maddq_w(...) vuctx_maddq_w(pPS2,__VA_ARGS__) 
#define vu_maddq_w_sf(...) vuctx_maddq_w_sf(pPS2,__VA_ARGS__) 
#define vu_msubx_y(...) vuctx_msubx_y(pPS2,__VA_ARGS__) 
#define vu_msuby_z(...) vuctx_msuby_z(pPS2,__VA_ARGS__) 
#define vu_msubz(...) vuctx_msubz(pPS2,__VA_ARGS__) 
#define vu_msubz_x(...) vuctx_msubz_x(pPS2,__VA_ARGS__) 
#define vu_msubz_y(...) vuctx_msubz_y(pPS2,__VA_ARGS__) 
#define vu_msubx_w_sf(...) vuctx_msubx_w_sf(pPS2,__VA_ARGS__) 
#define vu_msub_w(...) vuctx_msub_w(pPS2,__VA_ARGS__) 
#define vu_msub_w_sf(...) vuctx_msub_w_sf(pPS2,__VA_ARGS__) 
#define vu_msubw_xyz(...) vuctx_msubw_xyz(pPS2,__VA_ARGS__) 
#define vu_msubq_w(...) vuctx_msubq_w(pPS2,__VA_ARGS__) 
#define vu_msubq_w_sf(...) vuctx_msubq_w_sf(pPS2,__VA_ARGS__) 
#define vu_add(...) vuctx_add(pPS2,__VA_ARGS__) 
#define vu_add_w(...) vuctx_add_w(pPS2,__VA_ARGS__) 
#define vu_add_xyz(...) vuctx_add_xyz(pPS2,__VA_ARGS__) 
#define vu_addw_xyz(...) vuctx_addw_xyz(pPS2,__VA_ARGS__) 
#define vu_add_x(...) vuctx_add_x(pPS2,__VA_ARGS__) 
#define vu_add_y(...) vuctx_add_y(pPS2,__VA_ARGS__) 
#define vu_add_z(...) vuctx_add_z(pPS2,__VA_ARGS__) 
#define vu_add_xy(...) vuctx_add_xy(pPS2,__VA_ARGS__) 
#define vu_add_xz(...) vuctx_add_xz(pPS2,__VA_ARGS__) 
#define vu_addy_x(...) vuctx_addy_x(pPS2,__VA_ARGS__) 
#define vu_addy_z(...) vuctx_addy_z(pPS2,__VA_ARGS__) 
#define vu_addx_xyz(...) vuctx_addx_xyz(pPS2,__VA_ARGS__) 
#define vu_addx_y(...) vuctx_addx_y(pPS2,__VA_ARGS__) 
#define vu_addx_z(...) vuctx_addx_z(pPS2,__VA_ARGS__) 
#define vu_addx_w(...) vuctx_addx_w(pPS2,__VA_ARGS__) 
#define vu_addx_w_sf(...) vuctx_addx_w_sf(pPS2,__VA_ARGS__) 
#define vu_add_yz(...) vuctx_add_yz(pPS2,__VA_ARGS__) 
#define vu_addz_x(...) vuctx_addz_x(pPS2,__VA_ARGS__) 
#define vu_addz_y(...) vuctx_addz_y(pPS2,__VA_ARGS__) 
#define vu_addz_z(...) vuctx_addz_z(pPS2,__VA_ARGS__) 
#define vu_addx(...) vuctx_addx(pPS2,__VA_ARGS__) 
#define vu_addw(...) vuctx_addw(pPS2,__VA_ARGS__) 
#define vu_addw_x(...) vuctx_addw_x(pPS2,__VA_ARGS__) 
#define vu_addw_x_sf(...) vuctx_addw_x_sf(pPS2,__VA_ARGS__) 
#define vu_addq_x(...) vuctx_addq_x(pPS2,__VA_ARGS__) 
#define vu_addq_y(...) vuctx_addq_y(pPS2,__VA_ARGS__) 
#define vu_addq_z(...) vuctx_addq_z(pPS2,__VA_ARGS__) 
#define vu_mulq(...) vuctx_mulq(pPS2,__VA_ARGS__) 
#define vu_mulq_xy(...) vuctx_mulq_xy(pPS2,__VA_ARGS__) 
#define vu_mulq_xyz(...) vuctx_mulq_xyz(pPS2,__VA_ARGS__) 
#define vu_mulq_x(...) vuctx_mulq_x(pPS2,__VA_ARGS__) 
#define vu_mulq_w(...) vuctx_mulq_w(pPS2,__VA_ARGS__) 
#define vu_muli_w(...) vuctx_muli_w(pPS2,__VA_ARGS__) 
#define vu_sub(...) vuctx_sub(pPS2,__VA_ARGS__) 
#define vu_sub_xyz(...) vuctx_sub_xyz(pPS2,__VA_ARGS__) 
#define vu_sub_w(...) vuctx_sub_w(pPS2,__VA_ARGS__) 
#define vu_sub_w_sf(...) vuctx_sub_w_sf(pPS2,__VA_ARGS__) 
#define vu_subi_w_sf(...) vuctx_subi_w_sf(pPS2,__VA_ARGS__) 
#define vu_sub_xyz_sf(...) vuctx_sub_xyz_sf(pPS2,__VA_ARGS__) 
#define vu_subw_xyz(...) vuctx_subw_xyz(pPS2,__VA_ARGS__) 
#define vu_sub_x(...) vuctx_sub_x(pPS2,__VA_ARGS__) 
#define vu_sub_y(...) vuctx_sub_y(pPS2,__VA_ARGS__) 
#define vu_subw(...) vuctx_subw(pPS2,__VA_ARGS__) 
#define vu_subw_x(...) vuctx_subw_x(pPS2,__VA_ARGS__) 
#define vu_subw_x_sf(...) vuctx_subw_x_sf(pPS2,__VA_ARGS__) 
#define vu_sub_z(...) vuctx_sub_z(pPS2,__VA_ARGS__) 
#define vu_sub_xz(...) vuctx_sub_xz(pPS2,__VA_ARGS__) 
#define vu_sqrt(...) vuctx_sqrt(pPS2,__VA_ARGS__) 
#define vu_rsqrt(...) vuctx_rsqrt(pPS2,__VA_ARGS__) 
#define vu_div(...) vuctx_div(pPS2,__VA_ARGS__) 
#define vu_mini(...) vuctx_mini(pPS2,__VA_ARGS__) 
#define vu_mini_w(...) vuctx_mini_w(pPS2,__VA_ARGS__) 
#define vu_mini_xyz(...) vuctx_mini_xyz(pPS2,__VA_ARGS__) 
#define vu_mini_xz(...) vuctx_mini_xz(pPS2,__VA_ARGS__) 
#define vu_minix_x(...) vuctx_minix_x(pPS2,__VA_ARGS__) 
#define vu_miniw_xy(...) vuctx_miniw_xy(pPS2,__VA_ARGS__) 
#define vu_max(...) vuctx_max(pPS2,__VA_ARGS__) 
#define vu_max_xyz(...) vuctx_max_xyz(pPS2,__VA_ARGS__) 
#define vu_max_xz(...) vuctx_max_xz(pPS2,__VA_ARGS__) 
#define vu_maxx_x(...) vuctx_maxx_x(pPS2,__VA_ARGS__) 
#define vu_maxx_xy(...) vuctx_maxx_xy(pPS2,__VA_ARGS__) 
#define vu_opmula_xyz(...) vuctx_opmula_xyz(pPS2,__VA_ARGS__) 
#define vu_opmsub_xyz(...) vuctx_opmsub_xyz(pPS2,__VA_ARGS__) 
#define vu_opmsub_xyz_sf(...) vuctx_opmsub_xyz_sf(pPS2,__VA_ARGS__) 
#define vu_mr32(...) vuctx_mr32(pPS2,__VA_ARGS__) 
#define vu_mr32_w(...) vuctx_mr32_w(pPS2,__VA_ARGS__) 
#define vu_abs_xyz(...) vuctx_abs_xyz(pPS2,__VA_ARGS__) 
#define vu_abs_w(...) vuctx_abs_w(pPS2,__VA_ARGS__) 
#define vu_clipw_xyz_f(...) vuctx_clipw_xyz_f(pPS2,__VA_ARGS__) 
#define vu_clipw_xyz(...) vuctx_clipw_xyz(pPS2,__VA_ARGS__) 
#define vu_ftoi0(...) vuctx_ftoi0(pPS2,__VA_ARGS__) 
#define vu_ftoi12_xy(...) vuctx_ftoi12_xy(pPS2,__VA_ARGS__) 
#define vu_itof0(...) vuctx_itof0(pPS2,__VA_ARGS__) 
#define vu_itof0_xyz(...) vuctx_itof0_xyz(pPS2,__VA_ARGS__) 
#define vu_itof0_w(...) vuctx_itof0_w(pPS2,__VA_ARGS__) 
#define vu_nop() vuctx_nop(pPS2) 
#define vu_waitq() vuctx_waitq(pPS2) 
#define vu_lqi(...) vuctx_lqi(pPS2,__VA_ARGS__) 
#define vu_mfir_w(...) vuctx_mfir_w(pPS2,__VA_ARGS__) 
#define vu_lq_xyzw(...) vuctx_lq_xyzw(pPS2,__VA_ARGS__) 
#define vu_lq_x(...) vuctx_lq_x(pPS2,__VA_ARGS__) 
#define vu_lq_w(...) vuctx_lq_w(pPS2,__VA_ARGS__) 
#define vu_sqd(...) vuctx_sqd(pPS2,__VA_ARGS__) 
#define vu_sq_xyzw(...) vuctx_sq_xyzw(pPS2,__VA_ARGS__) 
#define vu_sq_xyz(...) vuctx_sq_xyz(pPS2,__VA_ARGS__) 
#define vu_sq_xzw(...) vuctx_sq_xzw(pPS2,__VA_ARGS__) 
#define vu_sq_yzw(...) vuctx_sq_yzw(pPS2,__VA_ARGS__) 
#define vu_sq_x(...) vuctx_sq_x(pPS2,__VA_ARGS__) 
#define vu_sq_y(...) vuctx_sq_y(pPS2,__VA_ARGS__) 
#define vu_sq_z(...) vuctx_sq_z(pPS2,__VA_ARGS__) 
#define vu_ilwr_x(...) vuctx_ilwr_x(pPS2,__VA_ARGS__) 
#define vu_ilwr_z(...) vuctx_ilwr_z(pPS2,__VA_ARGS__) 
#define vu_ilw_y(...) vuctx_ilw_y(pPS2,__VA_ARGS__) 
#define vu_ilw_z(...) vuctx_ilw_z(pPS2,__VA_ARGS__) 
#define vu_ilw_w(...) vuctx_ilw_w(pPS2,__VA_ARGS__) 
#define vu_isw_x(...) vuctx_isw_x(pPS2,__VA_ARGS__) 
#define vu_isw_y(...) vuctx_isw_y(pPS2,__VA_ARGS__) 
#define vu_isw_z(...) vuctx_isw_z(pPS2,__VA_ARGS__) 
#define vu_isw_w(...) vuctx_isw_w(pPS2,__VA_ARGS__) 
#define vu_rinit(...) vuctx_rinit(pPS2,__VA_ARGS__) 
#define vu_rnext() vuctx_rnext(&pPS2->vu0) 



//----------------------------------------------------------------------------
// CALL INSTRUCTIONS
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_callms( PS2CTX *pPS2, const int Imm15, vu_function const *function_table, int const function_count )
{
   // Compute function index
   const int index = Imm15>>4;
   bp_vu_assert( (Imm15 & 0x3) == 0 );
   bp_vu_assert( function_table );
   bp_vu_assert( index >= 0 );
   bp_vu_assert( index < function_count );
   
   // Call function
   bp_vu_assert( function_table[index] );
   (function_table[index])(pPS2);
}

//----------------------------------------------------------------------------
// STATUS FLAG INSTRUCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_update_status_flags( PS2CTX *pPS2, const FVECTOR* results, VU0_Flags* flags, const int x, const int y, const int z, const int w )
{
   vu_sanity_check(pPS2);

   // Clear status flags
   flags->status &= ~VU_STATUS_FLAGS;

   // Set flags
   if( x )
   {
      flags->mac &= ~(VU_MAC_FLAG_Sx | VU_MAC_FLAG_Zx | VU_MAC_FLAG_Ux | VU_MAC_FLAG_Ox);
      bp_vu_assert( BP_Float_Check(results->vx) );
      if( results->vx < 0 )   // sign
      {
         flags->mac |= VU_MAC_FLAG_Sx;
         flags->status |= VU_STATUS_FLAG_S | VU_STATUS_FLAG_SS;
      }
      else if( results->vx == 0 )  // zero
      {
         flags->mac |= VU_MAC_FLAG_Zx;
         flags->status |= VU_STATUS_FLAG_Z | VU_STATUS_FLAG_ZS;
      }
   }
   if( y )
   {
      flags->mac &= ~(VU_MAC_FLAG_Sy | VU_MAC_FLAG_Zy | VU_MAC_FLAG_Uy | VU_MAC_FLAG_Oy);
      bp_vu_assert( BP_Float_Check(results->vy) );
      if( results->vy < 0 )   // sign
      {
         flags->mac |= VU_MAC_FLAG_Sy;
         flags->status |= VU_STATUS_FLAG_S | VU_STATUS_FLAG_SS;
      }
      else if( results->vy == 0 )  // zero
      {
         flags->mac |= VU_MAC_FLAG_Zy;
         flags->status |= VU_STATUS_FLAG_Z | VU_STATUS_FLAG_ZS;
      }
   }
   if( z )
   {
      flags->mac &= ~(VU_MAC_FLAG_Sz | VU_MAC_FLAG_Zz | VU_MAC_FLAG_Uz | VU_MAC_FLAG_Oz);
      bp_vu_assert( BP_Float_Check(results->vz) );
      if( results->vz < 0 )   // sign
      {
         flags->mac |= VU_MAC_FLAG_Sz;
         flags->status |= VU_STATUS_FLAG_S | VU_STATUS_FLAG_SS;
      }
      else if( results->vz == 0 )  // zero
      {
         flags->mac |= VU_MAC_FLAG_Zz;
         flags->status |= VU_STATUS_FLAG_Z | VU_STATUS_FLAG_ZS;
      }
   }
   if( w )
   {
      flags->mac &= ~(VU_MAC_FLAG_Sw | VU_MAC_FLAG_Zw | VU_MAC_FLAG_Uw | VU_MAC_FLAG_Ow);
      bp_vu_assert( BP_Float_Check(results->vw) );
      if( results->vw < 0 )   // sign
      {
         flags->mac |= VU_MAC_FLAG_Sw;
         flags->status |= VU_STATUS_FLAG_S | VU_STATUS_FLAG_SS;
      }
      else if( results->vw == 0 )  // zero
      {
         flags->mac |= VU_MAC_FLAG_Zw;
         flags->status |= VU_STATUS_FLAG_Z | VU_STATUS_FLAG_ZS;
      }
   }
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_update_status_flags_xyz( PS2CTX *pPS2, const FVECTOR* results, VU0_Flags* flags )
{
   vu_update_status_flags( results, flags, 1, 1, 1, 0 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_update_status_flags_x( PS2CTX *pPS2, const FVECTOR* results, VU0_Flags* flags )
{
   vu_update_status_flags( results, flags, 1, 0, 0, 0 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_update_status_flags_w( PS2CTX *pPS2, const FVECTOR* results, VU0_Flags* flags )
{
   vu_update_status_flags( results, flags, 0, 0, 0, 1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_fsset( PS2CTX *pPS2, const int Imm12, VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   // Clear status sticky flags
   flags->status &= ~VU_STATUS_FLAGS_S;

   // Set stick flags
   flags->status |= (Imm12 & VU_STATUS_FLAGS_S);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_fsand( PS2CTX *pPS2, short* it, const int Imm12, const VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   *it = flags->status & Imm12;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_fmand( PS2CTX *pPS2, short* it, const short* is, const VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   *it = flags->mac & (*is);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_fcset( PS2CTX *pPS2, const int Imm24, VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   flags->clipping = Imm24;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_fcget( PS2CTX *pPS2, short* it, const VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   *it = flags->clipping & 0x0fff;
}

//----------------------------------------------------------------------------
// INTEGER INSTRUCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_iadd( PS2CTX *pPS2, short* id, const short* is, const short* it )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*id) = (*is) + (*it);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_iaddi( PS2CTX *pPS2, short* it, const short* is, const int Imm15 )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*it) = (*is) + Imm15;   
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_iaddiu( PS2CTX *pPS2, short* it, const short* is, const unsigned int Imm15 )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*it) = (*is) + Imm15;   
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_isubiu( PS2CTX *pPS2, short* it, const short* is, const unsigned int Imm15 )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*it) = (*is) - Imm15;   
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_isub( PS2CTX *pPS2, short* id, const short* is, const short* it )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*id) = (*is) - (*it);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_iand( PS2CTX *pPS2, short* id, const short* is, const short* it )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*id) = (*is) & (*it);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ior( PS2CTX *pPS2, short* id, const short* is, const short* it )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );

   (*id) = (*is) | (*it);
}

//----------------------------------------------------------------------------
// GENERAL INSTRUCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_loi( PS2CTX *pPS2, const float imm )
{
   vu_sanity_check(pPS2);

   pPS2->vu0.i = imm;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vx = fs->vx;
   ft->vy = fs->vy;
   ft->vz = fs->vz;
   ft->vw = fs->vw;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_xyzw(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   vu_move( ft, fs );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_xyz(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vx = fs->vx;
   ft->vy = fs->vy;
   ft->vz = fs->vz;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_xy(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vx = fs->vx;
   ft->vy = fs->vy;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_xz(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vx = fs->vx;
   ft->vz = fs->vz;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_yz(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vy = fs->vy;
   ft->vz = fs->vz;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_x(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vx = fs->vx;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_y(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vy = fs->vy;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_z(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vz = fs->vz;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_move_w(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vw = fs->vw;
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mul(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx * ft->vx;
   fd->vy = fs->vy * ft->vy;
   fd->vz = fs->vz * ft->vz;
   fd->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulx_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vx = ft->vx;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx * ft_vx;
   fd->vy = fs->vy * ft_vx;
   fd->vz = fs->vz * ft_vx;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulx_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx * ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulx_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vw = fs->vw * ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_muly_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vw = fs->vw * ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mul_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx * ft->vx;
   fd->vy = fs->vy * ft->vy;
   fd->vz = fs->vz * ft->vz;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mul_xy(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vx = fs->vx * ft->vx;
   fd->vy = fs->vy * ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vx) );
   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mul_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mul_xyz_sf(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx * ft->vx;
   fd->vy = fs->vy * ft->vy;
   fd->vz = fs->vz * ft->vz;

   bp_vu_assert( BP_Vec3_Check(fd) );

   vu_update_status_flags_xyz( fd, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mul_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx * ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulx(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vx = ft->vx;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx * ft_vx;
   fd->vy = fs->vy * ft_vx;
   fd->vz = fs->vz * ft_vx;
   fd->vw = fs->vw * ft_vx;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_muly(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vy = ft->vy;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vx = fs->vx * ft_vy;
   fd->vy = fs->vy * ft_vy;
   fd->vz = fs->vz * ft_vy;
   fd->vw = fs->vw * ft_vy;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulw(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vw = ft->vw;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx * ft_vw;
   fd->vy = fs->vy * ft_vw;
   fd->vz = fs->vz * ft_vw;
   fd->vw = fs->vw * ft_vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulz_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vw = fs->vw * ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulw_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx * ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulw_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx * ft->vw;
   fd->vy = fs->vy * ft->vw;
   fd->vz = fs->vz * ft->vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulw_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_adda_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   acc->vx = fs->vx + ft->vx;
   acc->vy = fs->vy + ft->vy;
   acc->vz = fs->vz + ft->vz;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_suba_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   acc->vx = fs->vx - ft->vx;
   acc->vy = fs->vy - ft->vy;
   acc->vz = fs->vz - ft->vz;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addax_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vx = fs->vx + ft->vx;
   acc->vy = fs->vy + ft->vx;
   acc->vz = fs->vz + ft->vx;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addaw_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vx = fs->vx + ft->vw;
   acc->vy = fs->vy + ft->vw;
   acc->vz = fs->vz + ft->vw;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mula_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vx = fs->vx * ft->vx;
   acc->vy = fs->vy * ft->vx;
   acc->vz = fs->vz * ft->vx;
   acc->vw = fs->vw * ft->vx;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulaw(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vx = fs->vx * ft->vw;
   acc->vy = fs->vy * ft->vw;
   acc->vz = fs->vz * ft->vw;
   acc->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulaw_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mula_xy(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx = fs->vx * ft->vx;
   acc->vy = fs->vy * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulaw_xy(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vx = fs->vx * ft->vw;
   acc->vy = fs->vy * ft->vw;

   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulai_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const float* i)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( i == &pPS2->vu0.i );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(*i) );

   acc->vx = fs->vx * (*i);
   acc->vy = fs->vy * (*i);
   acc->vz = fs->vz * (*i);

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulai_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const float* i)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( i == &pPS2->vu0.i );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*i) );

   acc->vw = fs->vw * (*i);

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mula(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   acc->vx = fs->vx * ft->vx;
   acc->vy = fs->vy * ft->vy;
   acc->vz = fs->vz * ft->vz;
   acc->vw = fs->vw * ft->vw;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vx = fs->vx * ft->vx;
   acc->vy = fs->vy * ft->vx;
   acc->vz = fs->vz * ft->vx;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_yz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vy = fs->vy * ft->vx;
   acc->vz = fs->vz * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(acc->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_x(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vx = fs->vx * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_y(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vy = fs->vy * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_z(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vz = fs->vz * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vw = fs->vw * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulax_w_sf(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vw = fs->vw * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vw) );

   vu_update_status_flags_w( acc, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulaq_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const float* q)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*q) );

   acc->vw = fs->vw * (*q);

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulay_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vw = fs->vw * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulay_xz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx = fs->vx * ft->vy;
   acc->vz = fs->vz * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulaz_xy(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   acc->vx = fs->vx * ft->vz;
   acc->vy = fs->vy * ft->vz;

   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulaw_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vx = fs->vx * ft->vw;
   acc->vy = fs->vy * ft->vw;
   acc->vz = fs->vz * ft->vw;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddx_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vz) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vz = acc->vz + (fs->vz * ft->vx);

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddx(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = acc->vx + (fs->vx * ft->vx);
   fd->vy = acc->vy + (fs->vy * ft->vx);
   fd->vz = acc->vz + (fs->vz * ft->vx);
   fd->vw = acc->vw + (fs->vw * ft->vx);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddaw_xy(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vy) );

   acc->vx += fs->vx * ft->vw;
   acc->vy += fs->vy * ft->vw;

   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddaw_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );
   bp_vu_assert( BP_Vec3_Check(acc) );

   acc->vx += fs->vx * ft->vw;
   acc->vy += fs->vy * ft->vw;
   acc->vz += fs->vz * ft->vw;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_madday(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx += fs->vx * ft->vy;
   acc->vy += fs->vy * ft->vy;
   acc->vz += fs->vz * ft->vy;
   acc->vw += fs->vw * ft->vy;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddaz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   acc->vx += fs->vx * ft->vz;
   acc->vy += fs->vy * ft->vz;
   acc->vz += fs->vz * ft->vz;
   acc->vw += fs->vw * ft->vz;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddax_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vw += fs->vw * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_madday_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vw += fs->vw * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddaz_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   acc->vw += fs->vw * ft->vz;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_madday_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx += fs->vx * ft->vy;
   acc->vy += fs->vy * ft->vy;
   acc->vz += fs->vz * ft->vy;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_madday_x(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx += fs->vx * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_madday_y(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vy += fs->vy * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_madday_z(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vz) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vz += fs->vz * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubax_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vw -= fs->vw * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubax_y(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vy -= fs->vy * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vy) );
}
//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubax_z(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vz) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vz -= fs->vz * ft->vx;

   bp_vu_assert( BP_Float_Check(acc->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubax(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   acc->vx -= fs->vx * ft->vx;
   acc->vy -= fs->vy * ft->vx;
   acc->vz -= fs->vz * ft->vx;
   acc->vw -= fs->vw * ft->vx;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubay(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx -= fs->vx * ft->vy;
   acc->vy -= fs->vy * ft->vy;
   acc->vz -= fs->vz * ft->vy;
   acc->vw -= fs->vw * ft->vy;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubay_x(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vx -= fs->vx * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msuba_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   acc->vw -= fs->vw * ft->vw;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubay_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   acc->vw -= fs->vw * ft->vy;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_msubaz_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   acc->vw -= fs->vw * ft->vz;

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddai_w(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const float* i )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( i == &pPS2->vu0.i );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*i) );

   acc->vw += fs->vw * (*i);

   bp_vu_assert( BP_Float_Check(acc->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddai_w_sf(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const float* i, VU0_Flags* flags )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( i == &pPS2->vu0.i );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*i) );

   acc->vw += fs->vw * (*i);

   bp_vu_assert( BP_Float_Check(acc->vw) );

   vu_update_status_flags_w( acc, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddi_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* i)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( fs != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(*i) );

   fd->vx = acc->vx + (fs->vx * (*i));
   fd->vy = acc->vy + (fs->vy * (*i));
   fd->vz = acc->vz + (fs->vz * (*i));

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddi_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* i)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( fs != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*i) );

   fd->vw = acc->vw + (fs->vw * (*i));

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_madd_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = acc->vx + (fs->vx * ft->vx);
   fd->vy = acc->vy + (fs->vy * ft->vy);
   fd->vz = acc->vz + (fs->vz * ft->vz);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddw_xy(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vx = acc->vx + (fs->vx * ft->vw);
   fd->vy = acc->vy + (fs->vy * ft->vw);

   bp_vu_assert( BP_Float_Check(fd->vx) );
   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddw_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vw = acc->vw + (fs->vw * ft->vw);

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_maddaz_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   acc->vx += fs->vx * ft->vz;
   acc->vy += fs->vy * ft->vz;
   acc->vz += fs->vz * ft->vz;

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddy_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vw = acc->vw + (fs->vw * ft->vy);

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddy_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vx = acc->vx + (fs->vx * ft->vy);

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = acc->vx + (fs->vx * ft->vz);
   fd->vy = acc->vy + (fs->vy * ft->vz);
   fd->vz = acc->vz + (fs->vz * ft->vz);
   fd->vw = acc->vw + (fs->vw * ft->vz);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddw(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float    ft_vw = ft->vw;
   const FVECTOR* acc  = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = acc->vx + (fs->vx * ft_vw);
   fd->vy = acc->vy + (fs->vy * ft_vw);
   fd->vz = acc->vz + (fs->vz * ft_vw);
   fd->vw = acc->vw + (fs->vw * ft_vw);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddw_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float    ft_vw = ft->vw;
   const FVECTOR* acc  = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = acc->vx + (fs->vx * ft_vw);
   fd->vy = acc->vy + (fs->vy * ft_vw);
   fd->vz = acc->vz + (fs->vz * ft_vw);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddw_xyz_sf(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags)
{
   const float    ft_vw = ft->vw;
   const FVECTOR* acc   = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = acc->vx + (fs->vx * ft_vw);
   fd->vy = acc->vy + (fs->vy * ft_vw);
   fd->vz = acc->vz + (fs->vz * ft_vw);

   bp_vu_assert( BP_Vec3_Check(fd) );

   vu_update_status_flags_xyz( fd, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float    ft_vz = ft->vz;
   const FVECTOR* acc   = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = acc->vx + (fs->vx * ft_vz);
   fd->vy = acc->vy + (fs->vy * ft_vz);
   fd->vz = acc->vz + (fs->vz * ft_vz);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = acc->vx + (fs->vx * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vy = acc->vy + (fs->vy * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vz = acc->vz + (fs->vz * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vw = acc->vw + (fs->vw * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddz_w_sf(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vw = acc->vw + (fs->vw * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vw) );

   vu_update_status_flags_w( fd, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddq_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vx = acc->vx + (fs->vx * (*q));
   fd->vy = acc->vy + (fs->vy * (*q));
   fd->vz = acc->vz + (fs->vz * (*q));

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddq_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vw = acc->vw + (fs->vw * (*q));

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maddq_w_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q, VU0_Flags* flags )
{
   FVECTOR res;
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*q) );

   res.vw = acc->vw + (fs->vw * (*q));

   bp_vu_assert( BP_Float_Check(res.vw) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vw = res.vw;
   }

   vu_update_status_flags_w( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubx_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vy = acc->vy - (fs->vy * ft->vx);

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msuby_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vz) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vz = acc->vz - (fs->vz * ft->vy);

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = acc->vx - (fs->vx * ft->vz);
   fd->vy = acc->vy - (fs->vy * ft->vz);
   fd->vz = acc->vz - (fs->vz * ft->vz);
   fd->vw = acc->vw - (fs->vw * ft->vz);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubz_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vx) );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = acc->vx - (fs->vx * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubz_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vy) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vy = acc->vy - (fs->vy * ft->vz);

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubx_w_sf(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vw = acc->vw - (fs->vw * ft->vx);

   bp_vu_assert( BP_Float_Check(fd->vw) );

   vu_update_status_flags_w( fd, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msub_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vw = acc->vw - (fs->vw * ft->vw);

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msub_w_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR res;
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   res.vw = acc->vw - (fs->vw * ft->vw);

   bp_vu_assert( BP_Float_Check(res.vw) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vw = res.vw;
   }

   vu_update_status_flags_w( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubw_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float    ft_vw = ft->vw;
   const FVECTOR* acc   = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(acc) );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = acc->vx - (fs->vx * ft_vw);
   fd->vy = acc->vy - (fs->vy * ft_vw);
   fd->vz = acc->vz - (fs->vz * ft_vw);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubq_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q)
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vw = acc->vw - (fs->vw * (*q));

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_msubq_w_sf(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q, VU0_Flags* flags )
{
   const FVECTOR* acc = &pPS2->vu0.acc;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(acc->vw) );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vw = acc->vw - (fs->vw * (*q));

   bp_vu_assert( BP_Float_Check(fd->vw) );

   vu_update_status_flags_w( fd, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx + ft->vx;
   fd->vy = fs->vy + ft->vy;
   fd->vz = fs->vz + ft->vz;
   fd->vw = fs->vw + ft->vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vw = fs->vw + ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx + ft->vx;
   fd->vy = fs->vy + ft->vy;
   fd->vz = fs->vz + ft->vz;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addw_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vw = ft->vw;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx + ft_vw;
   fd->vy = fs->vy + ft_vw;
   fd->vz = fs->vz + ft_vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx + ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vy = fs->vy + ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vz = fs->vz + ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_xy(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vx = fs->vx + ft->vx;
   fd->vy = fs->vy + ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vx) );
   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_xz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = fs->vx + ft->vx;
   fd->vz = fs->vz + ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vx) );
   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addy_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vx = fs->vx + ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addy_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vz = fs->vz + ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addx_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vx = ft->vx;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx + ft_vx;
   fd->vy = fs->vy + ft_vx;
   fd->vz = fs->vz + ft_vx;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addx_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vy = fs->vy + ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addx_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vz = fs->vz + ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addx_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vw = fs->vw + ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addx_w_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   res.vw = fs->vw + ft->vx;

   bp_vu_assert( BP_Float_Check(res.vw) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vw = res.vw;
   }

   vu_update_status_flags_w( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_add_yz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vy) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vy = fs->vy + ft->vy;
   fd->vz = fs->vz + ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vy) );
   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addz_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = fs->vx + ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addz_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vy = fs->vy + ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addz_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vz = fs->vz + ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addx(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vx = ft->vx;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx + ft_vx;
   fd->vy = fs->vy + ft_vx;
   fd->vz = fs->vz + ft_vx;
   fd->vw = fs->vw + ft_vx;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addw(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vw = ft->vw;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx + ft_vw;
   fd->vy = fs->vy + ft_vw;
   fd->vz = fs->vz + ft_vw;
   fd->vw = fs->vw + ft_vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addw_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx + ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addw_x_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   res.vx = fs->vx + ft->vw;

   bp_vu_assert( BP_Float_Check(res.vx) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vx = res.vx;
   }

   vu_update_status_flags_x( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addq_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vx = fs->vx + (*q);

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addq_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vy = fs->vy + (*q);

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_addq_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vz = fs->vz + (*q);

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulq(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vx = fs->vx * (*q);
   fd->vy = fs->vy * (*q);
   fd->vz = fs->vz * (*q);
   fd->vw = fs->vw * (*q);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulq_xy(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vx = fs->vx * (*q);
   fd->vy = fs->vy * (*q);

   bp_vu_assert( BP_Float_Check(fd->vx) );
   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulq_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vx = fs->vx * (*q);
   fd->vy = fs->vy * (*q);
   fd->vz = fs->vz * (*q);

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulq_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vx = fs->vx * (*q);

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_mulq_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* q )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*q) );

   fd->vw = fs->vw * (*q);

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_muli_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* i )
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( i == &pPS2->vu0.i );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*i) );

   fd->vw = fs->vw * (*i);

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx - ft->vx;
   fd->vy = fs->vy - ft->vy;
   fd->vz = fs->vz - ft->vz;
   fd->vw = fs->vw - ft->vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   fd->vx = fs->vx - ft->vx;
   fd->vy = fs->vy - ft->vy;
   fd->vz = fs->vz - ft->vz;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vw = fs->vw - ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vw) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_w_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   res.vw = fs->vw - ft->vw;

   bp_vu_assert( BP_Float_Check(res.vw) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vw = res.vw;
   }

   vu_update_status_flags_w( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_subi_w_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const float* i, VU0_Flags* flags )
{
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( i == &pPS2->vu0.i );
   bp_vu_assert( BP_Float_Check(fs->vw) );
   bp_vu_assert( BP_Float_Check(*i) );

   res.vw = fs->vw - (*i);

   bp_vu_assert( BP_Float_Check(res.vw) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vw = res.vw;
   }

   vu_update_status_flags_w( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_xyz_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   res.vx = fs->vx - ft->vx;
   res.vy = fs->vy - ft->vy;
   res.vz = fs->vz - ft->vz;

   bp_vu_assert( BP_Vec3_Check(&res) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vx = res.vx;
      fd->vy = res.vy;
      fd->vz = res.vz;
   }

   vu_update_status_flags_xyz( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_subw_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vw = ft->vw;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx - ft_vw;
   fd->vy = fs->vy - ft_vw;
   fd->vz = fs->vz - ft_vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vx) );

   fd->vx = fs->vx - ft->vx;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_y(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vy) );
   bp_vu_assert( BP_Float_Check(ft->vy) );

   fd->vy = fs->vy - ft->vy;

   bp_vu_assert( BP_Float_Check(fd->vy) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_subw(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   const float ft_vw = ft->vw;

   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx - ft_vw;
   fd->vy = fs->vy - ft_vw;
   fd->vz = fs->vz - ft_vw;
   fd->vw = fs->vw - ft_vw;

   bp_vu_assert( BP_Vec3_Check(fd) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_subw_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   fd->vx = fs->vx - ft->vw;

   bp_vu_assert( BP_Float_Check(fd->vx) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_subw_x_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(ft->vw) );

   res.vx = fs->vx - ft->vw;

   bp_vu_assert( BP_Float_Check(res.vx) );

   if( fd != &pPS2->vu0.vf[0] )
   {
      fd->vx = res.vx;
   }

   vu_update_status_flags_x( &res, flags );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_z(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vz = fs->vz - ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sub_xz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vx) );
   bp_vu_assert( BP_Float_Check(fs->vz) );
   bp_vu_assert( BP_Float_Check(ft->vx) );
   bp_vu_assert( BP_Float_Check(ft->vz) );

   fd->vx = fs->vx - ft->vx;
   fd->vz = fs->vz - ft->vz;

   bp_vu_assert( BP_Float_Check(fd->vx) );
   bp_vu_assert( BP_Float_Check(fd->vz) );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_sqrt(PS2CTX *pPS2, float* q, const float* ft)
{
   const float f = *ft;

   vu_sanity_check(pPS2);

   bp_vu_assert( q == &pPS2->vu0.q );
   bp_math_assert( BP_Float_Check(f) );

   // VU0 unit test verified that sqrt(abs(f)) is always computed
   *q = BP_Sqrt(BP_Fabsf(f));

   bp_vu_assert( BP_Float_Check(*q) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void vuctx_rsqrt(PS2CTX *pPS2, float* q, const float* fs, const float* ft)
{
   const float lhs = *fs;
   const float rhs = *ft;

   float s;

   vu_sanity_check(pPS2);

   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(lhs) );
   bp_vu_assert( BP_Float_Check(rhs) );
   
   // VU0 unit test verified that sqrt(abs(f)) is always computed
   s = BP_Sqrt(BP_Fabsf(rhs));

   bp_vu_assert( BP_Float_Check(s) );
   //if( s < FLT_EPSILON )
   if( s == 0.0f )
   {
      *q = 0.0f;
   }
   else
   {
      *q = lhs / s;
   }

   bp_vu_assert( BP_Float_Check(*q) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_div(PS2CTX *pPS2, float* q, const float* fs, const float* ft)
{
   const float lhs = *fs;
   const float rhs = *ft;

   vu_sanity_check(pPS2);

   bp_vu_assert( q == &pPS2->vu0.q );
   bp_vu_assert( BP_Float_Check(lhs) );
   bp_vu_assert( BP_Float_Check(rhs) );

   //if( fabs(rhs) < FLT_EPSILON )
   if( rhs == 0.0f )
   {
      *q = 0.0f;
   }
   else
   {
      *q = lhs / rhs;
   }

   bp_vu_assert( BP_Float_Check(*q) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mini(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   vu_sanity_check(pPS2);

   bp_math_assert( BP_Vec3_Check(fs) );
   bp_math_assert( BP_Vec3_Check(ft) );

   fd->vx = BP_Float_Min( fs->vx, ft->vx );
   fd->vy = BP_Float_Min( fs->vy, ft->vy );
   fd->vz = BP_Float_Min( fs->vz, ft->vz );
   fd->vw = BP_Float_Min( fs->vw, ft->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mini_w(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vw) );
   bp_math_assert( BP_Float_Check(ft->vw) );

   fd->vw = BP_Float_Min( fs->vw, ft->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mini_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Vec3_Check(fs) );
   bp_math_assert( BP_Vec3_Check(ft) );

   fd->vx = BP_Float_Min( fs->vx, ft->vx );
   fd->vy = BP_Float_Min( fs->vy, ft->vy );
   fd->vz = BP_Float_Min( fs->vz, ft->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mini_xz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vx) );
   bp_math_assert( BP_Float_Check(ft->vx) );
   bp_math_assert( BP_Float_Check(fs->vz) );
   bp_math_assert( BP_Float_Check(ft->vz) );

   fd->vx = BP_Float_Min( fs->vx, ft->vx );
   fd->vz = BP_Float_Min( fs->vz, ft->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_minix_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vx) );
   bp_math_assert( BP_Float_Check(ft->vx) );

   fd->vx = BP_Float_Min( fs->vx, ft->vx );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_miniw_xy(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vx) );
   bp_math_assert( BP_Float_Check(fs->vy) );
   bp_math_assert( BP_Float_Check(ft->vw) );

   fd->vx = BP_Float_Min( fs->vx, ft->vw );
   fd->vy = BP_Float_Min( fs->vy, ft->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_max(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Vec3_Check(fs) );
   bp_math_assert( BP_Vec3_Check(ft) );

   fd->vx = BP_Float_Max( fs->vx, ft->vx );
   fd->vy = BP_Float_Max( fs->vy, ft->vy );
   fd->vz = BP_Float_Max( fs->vz, ft->vz );
   fd->vw = BP_Float_Max( fs->vw, ft->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_max_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Vec3_Check(fs) );
   bp_math_assert( BP_Vec3_Check(ft) );

   fd->vx = BP_Float_Max( fs->vx, ft->vx );
   fd->vy = BP_Float_Max( fs->vy, ft->vy );
   fd->vz = BP_Float_Max( fs->vz, ft->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_max_xz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vx) );
   bp_math_assert( BP_Float_Check(fs->vz) );
   bp_math_assert( BP_Float_Check(ft->vx) );
   bp_math_assert( BP_Float_Check(ft->vz) );

   fd->vx = BP_Float_Max( fs->vx, ft->vx );
   fd->vz = BP_Float_Max( fs->vz, ft->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maxx_x(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vx) );
   bp_math_assert( BP_Float_Check(ft->vx) );

   fd->vx = BP_Float_Max( fs->vx, ft->vx );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_maxx_xy(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( fd != &pPS2->vu0.vf[0] );

   bp_math_assert( BP_Float_Check(fs->vx) );
   bp_math_assert( BP_Float_Check(fs->vy) );
   bp_math_assert( BP_Float_Check(ft->vx) );

   fd->vx = BP_Float_Max( fs->vx, ft->vx );
   fd->vy = BP_Float_Max( fs->vy, ft->vx );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_opmula_xyz(PS2CTX *pPS2, FVECTOR* acc, const FVECTOR* fs, const FVECTOR* ft)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( acc == &pPS2->vu0.acc );
   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   acc->vx = (fs->vy * ft->vz);
   acc->vy = (fs->vz * ft->vx);
   acc->vz = (fs->vx * ft->vy);

   bp_vu_assert( BP_Vec3_Check(acc) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_opmsub_xyz(PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft)
{
   FVECTOR* acc = &pPS2->vu0.acc;
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fd );
   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   res.vx = acc->vx - (fs->vy * ft->vz);
   res.vy = acc->vy - (fs->vz * ft->vx);
   res.vz = acc->vz - (fs->vx * ft->vy);

   bp_vu_assert( BP_Vec3_Check(&res) );

   fd->vx = res.vx;
   fd->vy = res.vy;
   fd->vz = res.vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_opmsub_xyz_sf( PS2CTX *pPS2, FVECTOR* fd, const FVECTOR* fs, const FVECTOR* ft, VU0_Flags* flags )
{
   FVECTOR* acc = &pPS2->vu0.acc;
   FVECTOR res;

   vu_sanity_check(pPS2);

   bp_vu_assert( acc != fd );
   bp_vu_assert( acc != fs );
   bp_vu_assert( acc != ft );
   bp_vu_assert( fd != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );
   bp_vu_assert( BP_Vec3_Check(ft) );

   res.vx = acc->vx - (fs->vy * ft->vz);
   res.vy = acc->vy - (fs->vz * ft->vx);
   res.vz = acc->vz - (fs->vx * ft->vy);

   bp_vu_assert( BP_Vec3_Check(&res) );

   fd->vx = res.vx;
   fd->vy = res.vy;
   fd->vz = res.vz;

   vu_update_status_flags_xyz( &res, flags );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mr32(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   FVECTOR res;
   res.vx = fs->vy;
   res.vy = fs->vz;
   res.vz = fs->vw;
   res.vw = fs->vx;

   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vx = res.vx;
   ft->vy = res.vy;
   ft->vz = res.vz;
   ft->vw = res.vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mr32_w(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );

   ft->vw = fs->vx;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_abs_xyz(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(fs) );

   ft->vx = BP_Fabsf(fs->vx);
   ft->vy = BP_Fabsf(fs->vy);
   ft->vz = BP_Fabsf(fs->vz);

   bp_vu_assert( BP_Vec3_Check(ft) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_abs_w(PS2CTX *pPS2, FVECTOR* ft, const FVECTOR* fs)
{
   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(fs->vw) );

   ft->vw = BP_Fabsf(fs->vw);

   bp_vu_assert( BP_Float_Check(ft->vw) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_clipw_xyz_f(PS2CTX *pPS2, const FVECTOR* ft, const FVECTOR* fs, VU0_Flags* vu_flags )
{
   const float pos_w = BP_Fabsf(fs->vw);
   const float neg_w = -pos_w;

   int flags = 0;

   vu_sanity_check(pPS2);

   bp_vu_assert( ft != &pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(ft) );

   if( ft->vx > pos_w ) flags |= 1<<0;
   else if( ft->vx < neg_w ) flags |= 1<<1;
   if( ft->vy > pos_w ) flags |= 1<<2;
   else if( ft->vy < neg_w ) flags |= 1<<3;
   if( ft->vz > pos_w ) flags |= 1<<4;
   else if( ft->vz < neg_w ) flags |= 1<<5;

   vu_flags->clipping = (vu_flags->clipping<<6) | flags;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_clipw_xyz(PS2CTX *pPS2, const FVECTOR* ft, const FVECTOR* fs )
{
   vu_clipw_xyz_f( ft, fs, &pPS2->vu0.flags[0] );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ftoi0(PS2CTX *pPS2, void* ft, const void* fs)
{
   IVECTOR* dst = (IVECTOR*)ft;
   const FVECTOR* src = (const FVECTOR*)fs;

   vu_sanity_check(pPS2);

   bp_vu_assert( dst != (IVECTOR*)&pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Vec3_Check(src) );

   dst->vx = (int)src->vx;
   dst->vy = (int)src->vy;
   dst->vz = (int)src->vz;
   dst->vw = (int)src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ftoi12_xy(PS2CTX *pPS2, void* ft, const void* fs)
{
   IVECTOR* dst = (IVECTOR*)ft;
   const FVECTOR* src = (const FVECTOR*)fs;

   vu_sanity_check(pPS2);

   bp_vu_assert( dst != (IVECTOR*)&pPS2->vu0.vf[0] );
   bp_vu_assert( BP_Float_Check(src->vx) );
   bp_vu_assert( BP_Float_Check(src->vy) );

   dst->vx = BP_Float_ToInt12( src->vx );
   dst->vy = BP_Float_ToInt12( src->vy );

   bp_vu_assert( BP_Float_Check(dst->vx) );
   bp_vu_assert( BP_Float_Check(dst->vy) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_itof0(PS2CTX *pPS2, void* ft, const void* fs)
{
         FVECTOR* dst = (FVECTOR*)ft;
   const IVECTOR* src = (const IVECTOR*)fs;

   vu_sanity_check(pPS2);

   bp_vu_assert( dst != &pPS2->vu0.vf[0] );

   dst->vx = (float)src->vx;
   dst->vy = (float)src->vy;
   dst->vz = (float)src->vz;
   dst->vw = (float)src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_itof0_xyz(PS2CTX *pPS2, void* ft, const void* fs)
{
         FVECTOR* dst = (FVECTOR*)ft;
   const IVECTOR* src = (const IVECTOR*)fs;

   vu_sanity_check(pPS2);

   bp_vu_assert( dst != &pPS2->vu0.vf[0] );

   dst->vx = (float)src->vx;
   dst->vy = (float)src->vy;
   dst->vz = (float)src->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_itof0_w(PS2CTX *pPS2, void* ft, const void* fs)
{
         FVECTOR* dst = (FVECTOR*)ft;
   const IVECTOR* src = (const IVECTOR*)fs;

   vu_sanity_check(pPS2);

   bp_vu_assert( dst != &pPS2->vu0.vf[0] );

   dst->vw = (float)src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_nop( PS2CTX *pPS2 )
{
   vu_sanity_check(pPS2);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_waitq( PS2CTX *pPS2 )
{
   vu_sanity_check(pPS2);
}

//----------------------------------------------------------------------------
// LOAD/STORE FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_lqi( PS2CTX *pPS2, FVECTOR* ft, short* is )
{
   const int offset = (*is);
   const FVECTOR* src = &pPS2->vu0.memory[offset].fv;
   
   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   ft->vx = src->vx;
   ft->vy = src->vy;
   ft->vz = src->vz;
   ft->vw = src->vw;

   (*is) = (*is) + 1;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_mfir_w( PS2CTX *pPS2, FVECTOR* ft, const short* is )
{
   int* dst = (int*)&ft->vw;

   vu_sanity_check(pPS2);

   *dst = *is;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_lq_xyzw( PS2CTX *pPS2, FVECTOR* ft, const int Imm11, const short* is )
{
   const int      offset = Imm11 + (*is);
   const FVECTOR* src    = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   ft->vx = src->vx;
   ft->vy = src->vy;
   ft->vz = src->vz;
   ft->vw = src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_lq_x( PS2CTX *pPS2, FVECTOR* ft, const int Imm11, const short* is )
{
   const int      offset = Imm11 + (*is);
   const FVECTOR* src    = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   ft->vx = src->vx;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_lq_w( PS2CTX *pPS2, FVECTOR* ft, const int Imm11, const short* is )
{
   const int      offset = Imm11 + (*is);
   const FVECTOR* src    = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   ft->vw = src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sqd( PS2CTX *pPS2, const FVECTOR* fs, short* it )
{
   int offset;
   FVECTOR* dst;

   (*it) = (*it) - 1;

   offset = (*it);
   dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vx = fs->vx;
   dst->vy = fs->vy;
   dst->vz = fs->vz;
   dst->vw = fs->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_xyzw( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vx = fs->vx;
   dst->vy = fs->vy;
   dst->vz = fs->vz;
   dst->vw = fs->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_xyz( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vx = fs->vx;
   dst->vy = fs->vy;
   dst->vz = fs->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_xzw( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vx = fs->vx;
   dst->vz = fs->vz;
   dst->vw = fs->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_yzw( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vy = fs->vy;
   dst->vz = fs->vz;
   dst->vw = fs->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_x( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vx = fs->vx;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_y( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vy = fs->vy;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_sq_z( PS2CTX *pPS2, const FVECTOR* fs, const int Imm11, const short* it )
{
   const int offset = Imm11 + (*it);
   FVECTOR* dst = &pPS2->vu0.memory[offset].fv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vz = fs->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ilwr_x( PS2CTX *pPS2, short* it, const short* is )
{
   const int      offset = *is;
   const IVECTOR* src    = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   *it = src->vx;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ilwr_z( PS2CTX *pPS2, short* it, const short* is )
{
   const int      offset = *is;
   const IVECTOR* src    = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   *it = src->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ilw_y( PS2CTX *pPS2, short* it, const int Imm11, const short* is )
{
   const int      offset = Imm11 + (*is);
   const IVECTOR* src    = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );
   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   *it = src->vy;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ilw_z( PS2CTX *pPS2, short* it, const int Imm11, const short* is )
{
   const int      offset = Imm11 + (*is);
   const IVECTOR* src    = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );
   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   *it = src->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_ilw_w( PS2CTX *pPS2, short* it, const int Imm11, const short* is )
{
   const int      offset = Imm11 + (*is);
   const IVECTOR* src    = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( it != &pPS2->vu0.vi[0] );
   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   *it = src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_isw_x( PS2CTX *pPS2, const short* it, const int Imm11, const short* is )
{
   const int offset = Imm11 + (*is);
   IVECTOR* dst = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vx = (*it) & 0x0000ffff;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_isw_y( PS2CTX *pPS2, const short* it, const int Imm11, const short* is )
{
   const int offset = Imm11 + (*is);
   IVECTOR* dst = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vy = (*it) & 0x0000ffff;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_isw_z( PS2CTX *pPS2, const short* it, const int Imm11, const short* is )
{
   const int offset = Imm11 + (*is);
   IVECTOR* dst = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vz = (*it) & 0x0000ffff;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_isw_w( PS2CTX *pPS2, const short* it, const int Imm11, const short* is )
{
   const int offset = Imm11 + (*is);
   IVECTOR* dst = &pPS2->vu0.memory[offset].iv;

   vu_sanity_check(pPS2);

   bp_vu_assert( ( offset >= 0 ) && ( offset < VU_MEM_COUNT ) );

   dst->vw = (*it) & 0x0000ffff;
}

//----------------------------------------------------------------------------
// VU0 Random
// NOTE: These functions use a very simple fast white noise function.
//       May need to revisit this if it's not "random" enough
//----------------------------------------------------------------------------

EXTERN_INLINE
void vuctx_rinit( PS2CTX *pPS2, const float f )
{
   vu_sanity_check(pPS2);

   bp_math_assert( BP_Float_Check(f) );

   pPS2->vu0.random = (unsigned short)(f*65535.0f);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float vuctx_rnext( PS2CTX *pPS2 )
{
   // VU_Users_Manual.pdf - page 187
   // RNEXT: Next Random Number
   // Returns range 1.0 -> 2.0
   const float min = 1.0f;
   const float max = 2.0f;

   vu_sanity_check(pPS2);

   // Advance to next pseudo random number in sequence
   if( pPS2->vu0.random & 1 )
   {
      pPS2->vu0.random = pPS2->vu0.random >> 1;
   }
   else
   {
      pPS2->vu0.random = ( pPS2->vu0.random >> 1 ) ^ 0xb400;
   }

   // Map from (0, 65535) to (min, max)
   return min + ( ( (float)pPS2->vu0.random * (1.0f/65535.0f) ) * ( max - min ) );
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_VU_H__
