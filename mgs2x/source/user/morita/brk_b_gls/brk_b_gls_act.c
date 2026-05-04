//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gls_act.c
   ガラス壊れ アクト

   1999/11/26 T. Morita
   $Id: brk_b_gls_act.c,v 1.1.1.3 2002/11/19 11:45:20 Yoshizawa1 Exp $
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
#include "def_dma.h"
#include "gameheader.h"

#include "brk_big_glass.h"


static int MoveObject( Work *work, FMATRIX *m, MOVE *p, int r_rot, float r_vel )
{
    static FVECTOR Gravity = { 0, -BRK_GRAVITY, 0, 0 } ;
    int flag = 0 ;

    if ( r_rot )
	p->rot_vx = p->rot_vx * (r_rot-1) / r_rot,
	p->rot_vy = p->rot_vy * (r_rot-1) / r_rot ;
    _sceVu0AddVector( &p->pos_v, &p->pos_v, &Gravity ) ;
    switch( BRK_BGLS_HzdCheck( work, &p->pos, &p->pos_v, BRK_BOUNCE+1.0f ) )
    {
    case 3:
    case 1:
	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, r_vel ) ;
	flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
        if ( flag == 2 )
            if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		p->pos.vy += 3.0f*rnd(), flag = 1 ;
	if ( flag > 6 )
	    p->rot_vx = irnd()&10 ? 200 : -200 ;
	else
	    p->rot_vx = ((p->rot_x>0 ? 1024 : -1024) - p->rot_x) / 16 ;
	break ;
    case 0:
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
#if 1
    p->rot_x += p->rot_vx, p->rot_y += p->rot_vy ;
    _sceVu0CopyMatrix( m, &BRK_BGLS_MatrixTable[(p->rot_x&4095)/256][(p->rot_y&4095)/256] ) ;
#else
    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x += p->rot_vx, p->rot_y += p->rot_vy ) ;
#endif
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

static int FallDownObject( Work *work, FMATRIX *m, MOVE *p, int r_rot, float r_vel )
{
    int r, flag ;

    if ( r_vel )
	p->rot_vy = (short)(p->rot_vy * (r_vel-1) / r_vel) ;
    r = ((p->rot_x>0 ? 1024 : -1024) - p->rot_x) ;
    r = p->rot_x + (p->rot_vx += r) ;
    if ( r == p->rot_x )
	flag = 1 ;
    else if ( (r > 1024 && p->rot_x <=  1024) || (r < 1024 && p->rot_x >=  1024) )
	r =  1024, flag = 1 ;
    else if ( (r >-1024 && p->rot_x <= -1024) || (r <-1024 && p->rot_x >= -1024) )
	r = -1024, flag = 1 ;
    else
	flag = 0 ;
#if 1
    p->rot_x = r ;
    p->rot_y += p->rot_vy ;
    _sceVu0CopyMatrix( m, &BRK_BGLS_MatrixTable[(p->rot_x&4095)/256][(p->rot_y&4095)/256] ) ;
#else
    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x = r, p->rot_y += p->rot_vy ) ;
#endif
    TransMatrix( m, &p->pos ) ;

    return flag ;
}


void BRK_BGLS_ActPieces( Work *work, int i, PIECE *p )
{
    int flag = 0, d ;

    for ( ; --i>=0 ; flag|=p->flag, p++ )
	if ( p->flag )
	    if ( (d = MoveObject( work, p->world, &p->mov, BRK_ROT_R, BRK_VEL_R )) == 1 )
		p->flag = 0 ;
    if ( !flag )
    {
	if ( !work->frac[0] && !work->frac[1] )
	    GV_Free( work->piece ), work->piece = NULL ;
	work->n_piece |= BRK_BGLS_INACTIVE ;
    }
}

void BRK_BGLS_StartActPieces( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width )
{
    PIECE  *p ;
    FVECTOR r ;

    work->n_piece &= ~BRK_BGLS_INACTIVE ;
    while( --i>=0 )
    {
	p = &work->piece[work->n_piece] ;
	r.vx = width*frnd() ;
	r.vy = width*frnd() ;
	r.vz = width*frnd() ;
	p->mov.rot_x = irnd()&4095 ;
	p->mov.rot_y = irnd()&4095 ;
	p->flag = 1 ;
	_sceVu0AddVector( &p->mov.pos  , pos  , &r ) ;
	_sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
	if ( ++work->n_piece >= BRK_N_PIECE )
	    work->n_piece = 0 ;
    }    
}

void BRK_BGLS_ActBreakDown( Work *work )
{
    int   i, j, det ;
    PART *p ;
    float fx, fy ;
    int flag = 3 ;
    FVECTOR *pos ;

    p = work->parts ;
    work->life += work->life_acs += BRK_SPEED ;
    for ( i=work->n_parts ; --i>=0 ; p++ )
    {
	switch( p->flag & 0xfff )
	{
	case 0:
	    pos = work->frac[0]->pos[0] ;
	    for ( j=work->n_frac ; --j>=0 ; pos+=5 )
	    {
		fx = p->mov.pos.vx - (pos[0].vx + pos[2].vx)*0.5f ;
		fy = p->mov.pos.vy - (pos[0].vy + pos[2].vy)*0.5f ;
		if ( work->life*work->life > fx*fx + fy*fy )
		{
		    p->flag &= 0xfffc, p->flag |= 1 ;
		    break ;
		}
	    }
	    break ;
	case 1:
	    det = MoveObject( work, &p->pos->world, &p->mov, 0, BRK_VEL_R ) ;
	    if ( p->flag & 0x8000 && det>0 )
	    {
		p->flag &= 0xfffc, p->flag |= 3 ;
		p->pos->color.vw = 0 ;
		BRK_BGLS_StartActPieces( work, 3, &p->mov.pos, &p->mov.pos_v, 2.0f ) ;
	    }
	    else if ( det & 1 )
		p->flag &= 0xfffc, p->flag |= 2 ;
	    break ;
	case 2:
	    if ( FallDownObject( work, &p->pos->world, &p->mov, BRK_ROT_R, BRK_VEL_R ) )
		p->flag &= 0xfffc, p->flag |= 3 ;
	    break ;
	}
	flag &= p->flag&0xfff ;
    }

    if ( (GV_Time % (0x3f*5/TIME_BASE)) == 0  )
	if ( work->se_time )
	    GM_SeSetMode( SD_A_GLSFALL1, &work->target.offset, GM_SEMODE_BOMB ), work->se_time-- ;
    if ( (GV_Time % (0x3f*5/TIME_BASE)) == 20*5/TIME_BASE )
	if ( work->se_time )
	    GM_SeSetMode( SD_A_GLSFALL2, &work->target.offset, GM_SEMODE_BOMB ), work->se_time-- ;

    if ( flag == 3 )
    {
	for ( i=work->n_comdl ; --i>=0 ; )
	    if ( work->comdl[i] && i&1 )
	    {
		DG_DequeueComdlObjs( work->comdl[i] ) ;
		DG_FreeComdl( work->comdl[i] ) ;
		work->comdl[i]=NULL ;
	    }
	GM_FreePrim2( work->frac[0] ), work->frac[0] = NULL ;
	GM_FreePrim2( work->frac[1] ), work->frac[1] = NULL ;
	GV_Free( work->parts ) ;
	work->parts = NULL ;
	work->act = NULL ;
    }
}

void BRK_BGLS_ActFracture( Work *work )
{
    PART *p ;
    int i, j, flag=0, se_flag=1 ;
    float fx, fy ;
    FVECTOR *pos ;

    work->life += work->life_acs += BRK_SPEED ;
    p = work->parts ;
    for ( i=work->n_parts ; --i>=0 ; flag |= !p->pos->color.vw, p++ )
	if ( !p->pos->color.vw )
	{
	    pos = work->frac[0]->pos[0] ;
	    for ( j=work->n_frac ; --j>=0 ; pos+=5 )
	    {
		fx = p->mov.pos.vx - (pos[0].vx + pos[2].vx)*0.5f ;
		fy = p->mov.pos.vy - (pos[0].vy + pos[2].vy)*0.5f ;
		if ( work->life*work->life > fx*fx + fy*fy )
		{
		    TransMatrix( &p->pos->world, &p->mov.pos ) ;
		    p->pos->color.vw = 4*BRK_SPEED, flag = 1 ;

		    if ( (GV_Time % 0x3) == /* work->se_time */ 0 && se_flag )
			GM_SeSetMode( SD_A_HIBI02, &p->mov.pos, GM_SEMODE_BOMB ), se_flag = 0 ;
		    break ;
		}
	    }
	}
	else if ( p->pos->color.vw < 80 )
	    p->pos->color.vw += 4*BRK_SPEED, flag = 1 ;

    if ( !flag )
    {
	FVECTOR EneFindPos = { 0.0f, 100.0f, -13500.0f, 1.0f } ;

	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
	work->objs = NULL ;
	DG_InvisiblePrim2( work->frac[1] ) ;
	DG_InvisiblePrim2( work->frac[0] ) ;
	work->life = work->life_acs = 0 ;
	work->act = BRK_BGLS_ActBreakDown ;

	work->se_time = 2 ;

	/* 敵兵は警戒モードに移行する */
	work->ene_find.type = EF_TYPE_LV3 ;
	_sceVu0CopyVector( &work->ene_find.pos, &EneFindPos ) ;
	GM_PutEneFind( &work->ene_find ) ;

	GM_SeSetMode( SD_A_GLASS01, &p->mov.pos, GM_SEMODE_BOMB ) ;
    }
}

void BRK_BGLS_ActFractureWeb( Work *work )
{
    FVECTOR  *v ;
    DG_PRIM2 *p ;
    int i ;

    if ( work->frac[0] && work->frac[1] && work->act != BRK_BGLS_ActBreakDown )
    {
	/* カメラに対し,裏表のプリミティブを決める */
	if ( DG_Chanls->eye.m[3][Z] > work->target.offset.vz )
	    DG_InvisiblePrim2( work->frac[0] ), DG_VisiblePrim2( p=work->frac[1] ) ;/*表*/
	else
	    DG_InvisiblePrim2( work->frac[1] ), DG_VisiblePrim2( p=work->frac[0] ) ;/*裏*/

	/* 優先防止用頂点をカメラに向けて進ませる （可視プリミディブのみ変更） */
	v = p->pos[0] ;
	for ( i=BRK_BGLS_N_SCAR ; --i>=0 ; v+=5 )
	{
	    v[4].vx = -50000.0f * DG_Chanls->eye.m[Z][X] + work->target.offset.vx ;
	    v[4].vz = -50000.0f * DG_Chanls->eye.m[Z][Z] + work->target.offset.vz ;
	}
    }
}
