//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   ケチャップの飛び散り
   2000/11/09 T.Morita

   $Id: efct_kchp.c,v 1.1.1.3 2002/11/19 11:45:38 Yoshizawa1 Exp $

*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"
#include "../brk_hzd/brk_hazard.h"
#include "../include/util.h"


#define BRK_N_STREAM        4
#define BRK_N_SPILL         4
#define BRK_N_VERTS 4


typedef struct work_t
{
    GV_ACT   actor  ;

    DG_PRIM2 *stream ;
    DG_PRIM2 *spill  ;
    int       flag   ;
    float     limit  ; /* ここまでは,伸ばせるという距離 */
} Work ;



static void Die( Work *work )
{
    if ( work->stream )
	GM_FreePrim2( work->stream ) ;
    if ( work->spill )
	GM_FreePrim2( work->spill ) ;
}

#define SPILL_VEL 0.85f
static void Act( Work *work )
{
    int             i ;
    FVECTOR  	   *p_cur, *p_prv ;
    //DG_PRIM2_UVRGB *u_cur, *u_prv ; /* stream用 */
    DG_PRIM2 *p ;

    if ( work->flag )
    {
	p = work->spill ;
	p_prv = p->pos[p->buffer_clock] ;
	DG_SwitchBuffPrim2( p ) ;
	p_cur = p->pos[p->buffer_clock] ;

	if ( (int)p_cur->vx==(int)p_prv->vx && (int)p_cur->vz==(int)p_prv->vz )
	    work->flag = 0 ;
	for ( i=BRK_N_VERTS ; --i>=0 ; p_cur++, p_prv++ )
	{
	    p_cur->vx = p_prv->vx + (p_prv->vx - p_cur->vx)*SPILL_VEL ;
	    p_cur->vz = p_prv->vz + (p_prv->vz - p_cur->vz)*SPILL_VEL ;
	}
    }
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static void InitPrimitive( DG_PRIM2 *prim, int id, u_int rgba,
			   float u_min, float u_max, float v_min, float v_max )
{
    DG_TEX *t ;
    DG_PRIM2_UVRGB *u0 = prim->uvrgb[0] ;
    DG_PRIM2_UVRGB *u1 = prim->uvrgb[1] ;

    DG_ConfigPrim2Tex( prim, t = DG_GetTexture( id ) ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    u0[0].u = u0[2].u = u1[0].u = u1[2].u = FTOI12( u_min * t->u_scale + t->u_offset ) ;
    u0[1].u = u0[3].u = u1[1].u = u1[3].u = FTOI12( u_max * t->u_scale + t->u_offset ) ;
    u0[0].v = u0[1].v = u1[0].v = u1[1].v = FTOI12( v_min * t->v_scale + t->v_offset ) ;
    u0[2].v = u0[3].v = u1[2].v = u1[3].v = FTOI12( v_max * t->v_scale + t->v_offset ) ;
    u0[0].q = u0[1].q = u0[2].q = u0[3].q =
	u1[0].q = u1[1].q = u1[2].q = u1[3].q = 4096 ;
    u0[0].f = u0[1].f = u1[0].f = u1[1].f = 0x8fff ;
    u0[2].f = u0[3].f = u1[2].f = u1[3].f = 0x0fff ;
    u0[0].r = u0[1].r = u0[2].r = u0[3].r = 
	u1[0].r = u1[1].r = u1[2].r = u1[3].r = (rgba >>  0) & 0xff ;
    u0[0].g = u0[1].g = u0[2].g = u0[3].g =
	u1[0].g = u1[1].g = u1[2].g = u1[3].g = (rgba >>  8) & 0xff ;
    u0[0].b = u0[1].b = u0[2].b = u0[3].b =
	u1[0].b = u1[1].b = u1[2].b = u1[3].b = (rgba >> 16) & 0xff ;
    u0[0].a = u0[1].a = u0[2].a = u0[3].a =
	u1[0].a = u1[1].a = u1[2].a = u1[3].a = (rgba >> 24) & 0xff ;
}

#define SPRT_FLAG (DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
static int GetResources( Work *work, HZD_BOX *hzd,
			 FVECTOR *o_pos, FVECTOR *o_dir,
			 int color, float up )
{
    int i ;
    FVECTOR   pos, vel, frc ;
    FVECTOR   v ;
    DG_PRIM2 *p ;
    static char *texname[] = { "chi01_alp", "chi04_alp", "chi03_alp", "chi05_alp" } ;

    if ( o_dir )
	frc.vx = o_dir->vx,   frc.vz = o_dir->vz   ;
    else
	frc.vx = frnd()*4.0f, frc.vz = frnd()*4.0f ; 
    frc.vy = 0.0f ;
    frc.vw = 1.0f ;
    _sceVu0Normalize( &frc, &frc ) ;

    /* まず下に当たりがあるかどうかを見る */
    _sceVu0ScaleVector( &v, &frc, -300.0f ); 
    _sceVu0SubVector( &pos, o_pos, &v ); 
    vel.vx = vel.vz = vel.vw = 0.0f ;
    vel.vy = -2000.0f ;
    if ( !(BRK_CheckHazard( hzd, &pos, &vel, &BRK_HZD_NoBounce, &DG_ZeroVector ) & 1) )
	return -1 ;

#if 0
    if ( !(p = work->spread = GM_MakePrim2( SPRT_FLAG, 1, BRK_N_VERTS )) )
	PERROR( "Stream DG_Prim2(Maybe no Memory) : NewKetchapSpread\n" ) ;
    InitPrimitive( work->stream, GV_StrCode( "splash03_alp" ), color,
		   0.0f, 1.0f, 0.0f, 0.5f ) ;
#endif

    if ( !(p = work->spill  = GM_MakePrim2( SPRT_FLAG, 1, BRK_N_VERTS )) )
	PERROR( "Spill DG_Prim2(Maybe no Memory) : NewKetchapSpread\n" ) ;
    InitPrimitive( work->spill, GV_StrCode( texname[irnd()&3]  ), color,
		   0.0f, 1.0f, 0.0f, 1.0f ) ;

    work->flag = 1 ;
    pos.vy += up /* + rnd()*3.0f*/ ;
    v.vy = v.vw = 0.0f ;
    for ( i=BRK_N_VERTS ; --i>=0 ; )
    {
	v.vx = (i&2 ? 1.0f : -1.0f ) * (i==1||i==2 ? -frc.vz : frc.vx) ;
	v.vz = (i&2 ? 1.0f : -1.0f ) * (i==1||i==2 ?  frc.vx : frc.vz) ;
	_sceVu0ScaleVector( &v, &v, i==BRK_N_VERTS-1 ? 80.0f : 30.0f ) ;
	_sceVu0AddVector( &work->spill->pos[p->buffer_clock][i], &pos, &v ) ;
	_sceVu0CopyVector( &work->spill->pos[p->buffer_clock^1][i], &pos ) ;
    }

    return 0 ;
}

void *NewKetchapSpread( HZD_BOX *hzd, FVECTOR *pos, FVECTOR *force,
			int color, float up )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, hzd, pos, force, color, up ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
