//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_spc_act.c
   ガラス壊れ アクト

   1999/11/26 T. Morita
   $Id: brk_spc_act.c,v 1.1.1.3 2002/11/19 11:45:45 Yoshizawa1 Exp $
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

#include "brk_speech.h"


#define BRK_SPC_SPREAD 40.0f

void BRK_SPC_StartActPieces( Work *work, FVECTOR *frc, int i )
{
    PIECE *p ;
    TARGET *t ;

    t = &work->target ;
    work->n_piece &= ~BRK_SPC_INACTIVE ;
    while( --i>=0 )
    {
	if ( work->n_piece >= BRK_N_PIECE )
	    work->n_piece = 0 ;
	p = &work->piece[work->n_piece++] ;
	p->rot_x  = 1024 ;
	p->rot_y  = irnd()&2047 ;
	p->rot_vx = irnd()&511 ;
	p->rot_vy = irnd()&511 ;
	p->pos.vx = t->size.vx*frnd() ;
	p->pos.vy = t->size.vy*frnd() ;
	p->pos.vz = t->size.vz*frnd() ;
	p->pos.vw = 1.0f ;

	p->pos_v.vx = 30.0f*frnd() ;
	p->pos_v.vy = 30.0f*frnd() ;
	p->pos_v.vz = 30.0f*frnd() ;
	_sceVu0AddVector( &p->pos, &p->pos, &t->offset ) ;
	_sceVu0ApplyMatrix( &p->pos, &t->world, &p->pos ) ;
	_sceVu0AddVector( &p->pos, &p->pos, &t->center ) ;
	p->pos_v.vx += frc->vx*0.005f ;
	p->pos_v.vz += frc->vz*0.005f ;
	p->comdl->color.vw = 64 ;
	RotateMatrixXY( p ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;
	p->flag = 1 ;
    }
}

void BRK_SPC_ActPieces( Work *work )
{
    int     i, flag=0 ;
    PIECE  *p ;
    FVECTOR v ;
    static FVECTOR Size    = { 10.0f, 10.0f, 10.0f } ;
    static FVECTOR Bounce  = { 1.48f, 1.48f, 1.48f, 0.0f } ;

    for ( i=BRK_N_PIECE, p=work->piece ; --i>=0 ; flag|=p->flag, p++ )
	if ( p->flag )
	{
	    p->rot_x += p->rot_vx ;
	    p->rot_y += p->rot_vy ;
	    p->rot_x &= 4095 ;
	    p->rot_y &= 4095 ;
	    RotateMatrixXY( p ) ;
	    p->pos_v.vy -= BRK_PIECE_GRAVITY ;
	    switch ( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &Bounce, &Size ) )
	    {
	    case 1:
	    case 3:
		_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
		_sceVu0ScaleVector( &v, &p->pos_v, 0.125f ) ;
		if ( !(int)v.vx && !(int)v.vy && !(int)v.vz )
		{
		    p->pos_v.vx = p->pos_v.vy = p->pos_v.vz = 0.0f ;
		    p->rot_vy = p->rot_vy * 15 /16 ;
		    if ( !p->rot_vx && !p->rot_vy )
			p->flag = 0 ;
		    else
			p->rot_vx = ( (p->rot_x>3072 ? 4096 :
				       p->rot_x>1024 ? 2048 :
				                       0) - p->rot_x)/2 ;
		}
		break ;
	    case 0:
		_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	    }
	    _sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;
	}
    if ( !flag )
	work->n_piece |= BRK_SPC_INACTIVE ;
}

