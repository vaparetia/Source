//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tv.c
   TV壊れ

   1999/12/13 T. Morita
   $Id: brk_tv_act.c,v 1.1.1.3 2002/11/19 11:45:49 Yoshizawa1 Exp $
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

#include "brk_tv.h"


void BRK_TV_ActMoveImage( Work *work )
{
    int      i ;
    DG_TEX  *t = DG_GetTexture( GV_StrCode( BRK_TV_SCREEN ) ) ;
    FVECTOR *p, *e, v ;
    DG_PRIM2_UVRGB *ua, *ui, *up ;

    work->scroll += work->speed ;
    if ( work->scroll < 0.0f )
	work->scroll = 0.0f, work->speed *= -1.0f ;
    else if ( work->scroll > 0.5f )
	work->scroll = 0.5f, work->speed *= -1.0f ;

    /* カメラの位置 */
    e = (FVECTOR *)&DG_Chanls->eye.m[W] ;

    /* スクリーンのスクロール */
    DG_SwitchBuffPrim2( work->scn_iv ) ;
    DG_SwitchBuffPrim2( work->scn_al ) ;
    DG_SwitchBuffPrim2( work->inv    ) ;
    ui = work->scn_iv->uvrgb[work->scn_iv->buffer_clock] ;
    ua = work->scn_al->uvrgb[work->scn_al->buffer_clock] ;
    up = work->inv->uvrgb[work->inv->buffer_clock] ;
    p  = work->scn_al->pos[work->scn_al->buffer_clock]   ;
    ua[0].u = ua[2].u = ui[0].u = ui[2].u = FTOI12( (work->scroll+0.0f) * t->u_scale + t->u_offset ) ;
    ua[1].u = ua[3].u = ui[1].u = ui[3].u = FTOI12( (work->scroll+0.5f) * t->u_scale + t->u_offset ) ;

    for ( i=4 ; --i>=0 ; p++, ua++, ui++, up++ )
    {
	_sceVu0SubVector( &v, p, e ) ;
	_sceVu0Normalize( &v, &v ) ;
	up->r =	up->g = 0 ;
	up->b = 128 ;
	up->a = 128 ;
	ui->r = ui->g = ui->b = 0 ;
	ua->g = 128 ; ua->r =  ua->b = (int)( sceVu0Sqrt( fpu_Abs( v.vx ) ) * 128.0f ) ;
	ua->a = (int)( sceVu0Sqrt( v.vx * v.vx + fpu_Abs( v.vy ) ) * 128.0f ) ;
    }
}

void BRK_TV_ActWipeOut( Work *work )
{
    int      i  ;
    DG_TEX  *t = DG_GetTexture( GV_StrCode( BRK_TV_WIPE ) ) ;
    FVECTOR *nxt_p, *prv_p  ;
    float    p ;
    DG_PRIM2_UVRGB *nxt_u, *prv_u ;

    prv_u = work->wipe->uvrgb[work->wipe->buffer_clock] ;
    prv_p = work->wipe->pos  [work->wipe->buffer_clock] ;
    DG_SwitchBuffPrim2( work->wipe ) ;
    nxt_u = work->wipe->uvrgb[work->wipe->buffer_clock] ;
    nxt_p = work->wipe->pos  [work->wipe->buffer_clock] ;

    for ( i=4 ; --i>=0 ; prv_p++, nxt_p++, prv_u++, nxt_u++ )
    {
	p = work->hit.vy + (i&2 ? 1.0f : -1.0f) * work->count * (BRK_TV_WIPE_SPEED * 0.5f) ;
	if ( p < work->def->ly )
	{
	    nxt_p->vy = work->def->ly ;
	    p = 0.0f + (work->def->ly - p) / work->count / BRK_TV_WIPE_SPEED ;
	    nxt_u->v = FTOI12( p * t->v_scale + t->v_offset ) ;
	}
	else if ( p > work->def->uy )
	{
	    nxt_p->vy = work->def->uy ;
	    p = 1.0f - (p - work->def->uy) / work->count / BRK_TV_WIPE_SPEED ;
	    nxt_u->v = FTOI12( p * t->v_scale + t->v_offset ) ;
	}
	else
	    nxt_p->vy = p ;

	p = work->hit.vz + (i&1 ? 1.0f : -1.0f) * work->count * (BRK_TV_WIPE_SPEED * 0.5f) ;
	if ( p < work->def->lz )
	{
	    nxt_p->vz = work->def->lz ;
	    p = 0.0f + (work->def->lz - p) / work->count / BRK_TV_WIPE_SPEED ;
	    nxt_u->u = FTOI12( p  * t->u_scale + t->u_offset ) ;
	}
	else if ( p > work->def->uz )
	{
	    nxt_p->vz = work->def->uz ;
	    p = 1.0f - (p - work->def->uz) / work->count / BRK_TV_WIPE_SPEED ;
	    nxt_u->u = FTOI12( p * t->u_scale + t->u_offset ) ;
	}
	else
	    nxt_p->vz = p ;
    }
}
