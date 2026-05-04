//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_wine.c
   瓶壊れ

   1999/12/02 T. Morita
   $Id: brk_wine.c,v 1.1.1.3 2002/11/19 11:45:24 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


#define BRK_N_DRIP          4
#define BRK_N_STREAM        6
#define BRK_N_SPILL         4

#define BRK_DRIP_GRAVITY    1.0f
#define BRK_STREAM_GRAVITY  4

#define BRK_WIN_DRIPSTART   200
#define BRK_WIN_VANISHSTART 30

#define BRK_BTL_WINE_SPD    0.98f


typedef struct work_t
{
    GV_ACT   actor  ;

    FMATRIX   world  ;

    DG_PRIM2 *drip   ;
    DG_PRIM2 *stream ;
    DG_PRIM2 *spill  ;

    float     radius ;
    int       alpha  ;

    
} Work ;



static void Die( Work *work )
{
    if ( work->drip )
	GM_FreePrim2( work->drip   ) ;
    if ( work->stream )
	GM_FreePrim2( work->stream ) ;
    if ( work->spill )
	GM_FreePrim2( work->spill ) ;
}

static void Act( Work *work )
{
    int      i    ;
    int      flag ;
    FVECTOR        *p_cur, *p_prv ;
    DG_PRIM2_UVRGB *u_cur, *u_prv ;

    if ( work->radius < 0.0f )
	   return ;
    if ( work->radius >= work->spill->pos[0]->vw )
    {
      _sceVu0ScaleVector( (FVECTOR*)&work->spill->as_world.m[3],
			          (FVECTOR*)&work->world.m[3],
			          1.0f/(work->spill->pos[0]->vw += 0.1f) ) ;
    }
    else if ( work->stream )
    {
	if ( work->alpha > 0 )
	{
	    u_prv = work->stream->uvrgb[work->stream->buffer_clock] ;
	    p_prv = work->stream->pos  [work->stream->buffer_clock] ;
	    DG_SwitchBuffPrim2( work->stream ) ;
	    u_cur = work->stream->uvrgb[work->stream->buffer_clock] ;
	    p_cur = work->stream->pos  [work->stream->buffer_clock] ;

	    if ( --work->alpha < 128 )
		for ( i=BRK_N_STREAM ; --i>=0 ; )
		{
		    p_cur->vx = p_prv->vx * BRK_BTL_WINE_SPD ;
		    u_cur->a  = work->alpha ;
		    u_cur++, p_cur++, u_prv++, p_prv++ ;
		}
	    else
	    {
		u_cur->a  = 128 ;
		p_cur[1].vw =  p_prv[1].vw - BRK_STREAM_GRAVITY ;
		p_cur[4].vy += p_prv[4].vy + p_cur[1].vw ;
		p_cur[5].vy += p_prv[5].vy + p_cur[1].vw ;
	    }
	}
	else
	{
	    flag = 1 ;

	    u_prv = work->drip->uvrgb[work->drip->buffer_clock] ;
	    p_prv = work->drip->pos  [work->drip->buffer_clock] ;
	    DG_SwitchBuffPrim2( work->drip ) ;
	    u_cur = work->drip->uvrgb[work->drip->buffer_clock] ;
	    p_cur = work->drip->pos  [work->drip->buffer_clock] ;

	    for ( i=BRK_N_DRIP ; --i>=0 ; u_cur+=2, p_cur+=2, u_prv+=2, p_prv+=2 )
		if ( p_prv[1].vw > 0 )
		{
		    flag = 0 ;
		    p_cur[1].vw = p_prv[1].vw - 1.0f ;
		    if ( p_cur[1].vw < BRK_WIN_DRIPSTART )
		    {
			p_cur[1].vy  = p_prv[0].vy ;
			p_cur[0].vw  = p_prv[0].vw + BRK_DRIP_GRAVITY ;
			p_cur[0].vy  = p_prv[0].vy - p_cur[0].vw ;
			u_cur[0].a  = DG_FTOI( p_cur[1].vw );
		    }
		}
	    if ( flag )
		work->radius = -1.0f ;
	}
    }

}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

static void InitStreamPrim( DG_PRIM2 *prim, int id, u_int rgba )
{
    DG_TEX *t ;
    DG_PRIM2_UVRGB *u0, *u1 ;

    u0 = prim->uvrgb[0] ;
    u1 = prim->uvrgb[1] ;

    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    DG_ConfigPrim2Tex( prim, t = DG_GetTexture( id ) ) ;
    u0[0].u = u0[2].u = u0[4].u =
	u1[0].u = u1[2].u = u1[4].u = FTOI12( 0.0f*t->u_scale + t->u_offset ) ;
    u0[1].u = u0[3].u = u0[5].u =
	u1[1].u = u1[3].u = u1[5].u = FTOI12( 1.0f*t->u_scale + t->u_offset ) ;
    u0[0].v = u0[1].v =
	u1[0].v = u1[1].v = FTOI12( 0.0f*t->v_scale + t->v_offset ) ;
    u0[2].v = u0[3].v =
	u1[2].v = u1[3].v = FTOI12( 0.5f*t->v_scale + t->v_offset ) ;
    u0[4].v = u0[5].v =
	u1[4].v = u1[5].v = FTOI12( 1.0f*t->v_scale + t->v_offset ) ;
    u0[0].q = u0[1].q = u0[2].q = u0[3].q = u0[4].q = u0[5].q =
	u1[0].q = u1[1].q = u1[2].q = u1[3].q = u1[4].q = u1[5].q = 4096 ;
    u0[0].f = u0[1].f =
	u1[0].f = u1[1].f = 0x8fff ;
    u0[2].f = u0[3].f = u0[4].f = u0[5].f =
	u1[2].f = u1[3].f = u1[4].f = u1[5].f = 0x0fff ;
    u1[0].r = u1[1].r = u1[2].r =
	u1[3].r = u1[4].r = u1[5].r =
	u0[0].r = u0[1].r = u0[2].r =
	u0[3].r = u0[4].r = u0[5].r = (rgba>> 0)&0xff ;
    u1[0].g = u1[1].g = u1[2].g =
	u1[3].g = u1[4].g = u1[5].g =	     	 
	u0[0].g = u0[1].g = u0[2].g =
	u0[3].g = u0[4].g = u0[5].g = (rgba>> 8)&0xff ;
    u1[0].b = u1[1].b = u1[2].b =
	u1[3].b = u1[4].b = u1[5].b =	     	 
	u0[0].b = u0[1].b = u0[2].b =
	u0[3].b = u0[4].b = u0[5].b = (rgba>>16)&0xff ;
    u1[0].a = u1[1].a = u1[2].a =
	u1[3].a = u1[4].a = u1[5].a =	     	 
	u0[0].a = u0[1].a = u0[2].a =
	u0[3].a = u0[4].a = u0[5].a = (rgba>>24)&0xff ;
}

static void InitSpillPrim( DG_PRIM2 *prim, int id, u_int rgba )
{
    DG_TEX *t ;
    DG_PRIM2_UVRGB *u = prim->uvrgb[0] ;

    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    DG_ConfigPrim2Tex( prim, t = DG_GetTexture( id ) ) ;
    u[0].u = u[2].u = FTOI12( 0.0F * t->u_scale + t->u_offset ) ;
    u[1].u = u[3].u = FTOI12( 1.0F * t->u_scale + t->u_offset ) ;
    u[0].v = u[1].v = FTOI12( 0.0F * t->v_scale + t->v_offset ) ;
    u[2].v = u[3].v = FTOI12( 1.0F * t->v_scale + t->v_offset ) ;
    u[0].q = u[1].q = u[2].q = u[3].q =  4096 ;
    u[0].f = u[1].f = 0x8fff ;
    u[2].f = u[3].f = 0x0fff ;
    u[0].r = u[1].r = u[2].r = u[3].r = (rgba >>  0) & 0xff ;
    u[0].g = u[1].g = u[2].g = u[3].g = (rgba >>  8) & 0xff ;
    u[0].b = u[1].b = u[2].b = u[3].b = (rgba >> 16) & 0xff ;
    u[0].a = u[1].a = u[2].a = u[3].a = (rgba >> 24) & 0xff ;
}


static void InitLinePrim( DG_PRIM2 *prim, int i, int n_verts, u_int rgba )
{
    int j ;
    DG_PRIM2_UVRGB *u0, *u1 ;

    u0 = prim->uvrgb[0] ;
    u1 = prim->uvrgb[1] ;

    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    for ( ; --i>=0 ; )
        for ( j=0 ; j<n_verts ; j++, u0++, u1++ )
        {
            u1->f = u0->f = j ? 0x0fff : 0x8fff ;
            u1->r = u0->r =  (rgba >>  0) & 0xff ;
            u1->g = u0->g =  (rgba >>  8) & 0xff ;
            u1->b = u0->b =  (rgba >> 16) & 0xff ;
            u1->a = u0->a = ((rgba >> 24) & 0xff)*j/n_verts ;
        }
}

static int GetResources( Work *work, FMATRIX *world, float width, float limit )
{
    int i ;
    static FVECTOR stream_shape[] = {
	{-30, 0,  0, 1}, { 30, 0,  0, 1}, {-30, 0, 50, 1}, { 30, 0, 50, 1},
	{-30, 0, 50, 1}, { 30, 0, 50, 1},
    } ;
    static FVECTOR spill_shape[] = {
	{-30, 0, -30, 1}, { 30, 0, -30, 1}, {-30, 0, 30, 1}, { 30, 0, 30, 1},
    } ;
    static short start_frm[] = {  0 + BRK_WIN_DRIPSTART,
				  5 + BRK_WIN_DRIPSTART,
				 20 + BRK_WIN_DRIPSTART,
				 45 + BRK_WIN_DRIPSTART } ;
    static short start_posx[] = { 15, 15, -14, 0 } ;


    work->radius = (limit<0.0f ? fpu_Abs( width ) : limit) / 30.0f ;
    work->alpha  = 128+BRK_WIN_VANISHSTART ;
    work->world  = *world ;

    work->spill = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|
				DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF,
				1, BRK_N_SPILL ) ;
    
    work->spill->flag |= DG_PRIM_AS_CUSTOMWORLD;
    
    if ( !work->spill )
        return -1 ;
    InitSpillPrim( work->spill, 4587425/*splash03_alp*/, 0x700a0040 ) ;
    work->spill->as_world = *world ;
    memcpy( work->spill->pos[0], spill_shape,
	    sizeof(FVECTOR)*BRK_N_SPILL ) ;

    if ( limit >= fpu_Abs( width ) )
    {
	work->stream = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				     1, BRK_N_STREAM ) ;
	if ( !work->stream )
	    return -1 ;
	InitStreamPrim( work->stream, 4587425/*splash03_alp*/, 0x700a0040 ) ;
   work->stream->flag |= DG_PRIM_AS_CUSTOMWORLD;
	work->stream->as_world = *world ;

	memcpy( work->stream->pos[0], stream_shape,
		sizeof(FVECTOR)*BRK_N_STREAM ) ;
	work->stream->pos[0][2].vz = work->stream->pos[0][3].vz = 
	    work->stream->pos[0][4].vz = work->stream->pos[0][5].vz = width ;
	memcpy( work->stream->pos[1], work->stream->pos[0],
		sizeof(FVECTOR)*BRK_N_STREAM ) ;


	work->drip = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA,
				   BRK_N_DRIP, 2 ) ;
	if ( !work->drip )
	    return -1 ;
	InitLinePrim( work->drip, BRK_N_DRIP, 2, 0x700a0040 ) ;
   work->drip->flag |= DG_PRIM_AS_CUSTOMWORLD;
   work->drip->as_world = *world ;

	for ( i=BRK_N_DRIP ; --i>=0 ; )
	{
	    work->drip->pos[0][i*2].vx = work->drip->pos[0][i*2+1].vx = start_posx[i] ;
	    work->drip->pos[0][i*2].vz = work->drip->pos[0][i*2+1].vz = width ;
	    work->drip->pos[0][i*2].vy = work->drip->pos[0][i*2+1].vy = 0     ;
	    work->drip->pos[0][i*2].vw = 0 ;
	    work->drip->pos[0][i*2+1].vw = start_frm[i] ;
	}
	memcpy( work->drip->pos[0], work->drip->pos[1],
		sizeof(FVECTOR)*BRK_N_DRIP*2 ) ;
    }

    GM_SeSetMode( SD_A_KOBORE01, (FVECTOR*)&world->m[3], GM_SEMODE_NORMAL ) ;

    return 0 ;
}

void *NewWineStream( FMATRIX *world, float width )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, world, width, -1.0 ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}



void *NewWineStreamLimit( FMATRIX *world, float width, float limit )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, world, width, limit ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}


