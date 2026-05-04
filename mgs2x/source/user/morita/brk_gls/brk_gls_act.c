//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gls_act.c
   ガラス壊れ アクト

   1999/11/26 T. Morita
   $Id: brk_gls_act.c,v 1.1.1.3 2002/11/19 11:45:29 Yoshizawa1 Exp $
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

#include "brk_glass.h"


#define BRK_GLS_SPREAD 40.0f

#if 0
void BRK_GLS_StartActPieces( Work *work, FVECTOR *pos, FVECTOR *frc, int i )
{
    PIECE *p ;

    if ( !work->lpiece_nm || !work->spiece_nm )
	return ;

    work->n_piece &= ~BRK_GLS_INACTIVE ;
    while( --i>=0 )
    {
	if ( work->n_piece >= BRK_N_PIECE )
	    work->n_piece = 0 ;
	p = &work->piece[work->n_piece++] ;
	p->rot_x  = 0  ;
	p->rot_y  = irnd()&2047 ;
	p->rot_vx = 512 ;
	p->rot_vy = 0  ;
	p->pos.vx = pos->vx + (p->pos_v.vx = BRK_GLS_SPREAD*frnd()) ;
	p->pos.vy = pos->vy + (p->pos_v.vy = BRK_GLS_SPREAD*frnd()) ;
	p->pos.vz = pos->vz + (p->pos_v.vz = BRK_GLS_SPREAD*frnd()) ;
	p->pos_v.vx += frc->vx*0.005f ;
	p->pos_v.vz += frc->vz*0.005f ;
	p->comdl->color.vw = 64 ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;
	p->flag = 1 ;
    }
}

void BRK_GLS_ActPieces( Work *work )
{
    int     i, flag=0 ;
    PIECE  *p ;
    FVECTOR v ;
    static FVECTOR Size    = { 10.0f, 10.0f, 10.0f } ;
    static FVECTOR Bounce  = { 1.48f, 1.48f, 1.48f, 0.0f } ;

    for ( i=BRK_N_PIECE, p=work->piece ; --i>=0 ; flag|=p->flag, p++ )
	if ( p->flag )
	{
	    RotateMatrixXY( &p->comdl->world,
			    p->rot_x+=p->rot_vx,
			    p->rot_y+=p->rot_vy ) ;
	    p->pos_v.vy -= BRK_PIECE_GRAVITY ;
	    switch ( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &Bounce, &Size ) )
	    {
	    case 1:
	    case 3:
		_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
		_sceVu0ScaleVector( &v, &p->pos_v, 0.125f ) ;
		if ( !(int)v.vx && !(int)v.vy && !(int)v.vz )
		    p->rot_x = 0, p->flag = 0 ;
		break ;
	    case 0:
		_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	    }
	    _sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;
	}
    if ( !flag )
	work->n_piece |= BRK_GLS_INACTIVE ;
}
#endif

void BRK_GLS_ActGlassEach( DG_PRIM2 *p, int i )
{
    DG_PRIM2_UVRGB *u_prv, *u_cur ;
    FVECTOR        *p_prv, *p_cur ;

    u_prv = p->uvrgb[p->buffer_clock] ;
    p_prv = p->pos  [p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    u_cur = p->uvrgb[p->buffer_clock] ;
    p_cur = p->pos  [p->buffer_clock] ;

    while( --i>=0 )
    {
	(u_cur++)->a = (u_prv++)->a ;
	_sceVu0CopyVector( p_cur++, p_prv++ ) ;
    }
}

void BRK_GLS_ActGlass( Work *work )
{
    BRK_GLS_ActGlassEach( work->broken, work->n_poly *  BRK_GLS_NVERTS*2  ) ;
    BRK_GLS_ActGlassEach( work->thick , work->n_poly * (BRK_GLS_NVERTS+1) ) ;
    work->flag-- ;
}

#ifdef KP_XBOX
void BRK_GLS_ActFlipTheFace( Work *work )
{
	if ( 0 ){
	}
}
#endif
