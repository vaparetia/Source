//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_plt_act.c
   皿壊れ アクト

   2000/04/25 T. Morita
   $Id: brk_plt_act.c,v 1.1.1.3 2002/11/19 11:45:40 Yoshizawa1 Exp $
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

#include "brk_plate.h"
#include "../brk_utl/brk_utl.x"


/***

  アクト補助関数

  ***/
/* MOVE 構造体より マトリックスの作成 */
static inline void MakeMoveMatrix( FMATRIX *m, MOVE *p )
{
    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    TransMatrix( m, &p->pos ) ;
}


/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( Work *work, FMATRIX *m, MOVE *p, DG_MDL *mdl, int r_rot, float r_vel )
{
    static FVECTOR BRK_PLT_Bounce  = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR size = { BRK_DUST_SPHERE, 3.0f, BRK_DUST_SPHERE, 0 } ;
    int flag = 0 ;

    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( mdl )
	BRK_UTL_SizeOfMDL( mdl, m, &size ) ;

    p->pos_v.vy -= BRK_GRAVITY ;
    switch( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v,
			     &BRK_PLT_Bounce, &size ) )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, r_vel ) ;
        flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
        if ( flag == 2 )
            if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
	    {
                p->pos.vy += 3.0f*rnd(), flag = 1 ;
	    }
        if ( flag > 6 )
            p->rot_vx = irnd()&10 ? 200 : -200 ;
        else
            p->rot_vx = (( !(p->rot_x/1024) ? 0 : p->rot_x>0 ? 2048 : -2048) - p->rot_x) / 16 ;
	/* 重心を見て跳ね返り方向を変えてしまう */
	if ( mdl )
	    if ( BRK_UTL_PutCenterHazard( work->hzd, &p->pos, &p->pos_v, 1000.0f, 25.0f ) )
	    {
		p->pos_v.vy = 30.0f ;
		p->rot_vx = irnd()&10 ? 200 : -200 ;
		flag++ ;
	    }

        break ;
    case 0:
	p->rot_x += p->rot_vx = p->rot_vx * (r_rot-1) / r_rot ;
	p->rot_y += p->rot_vy = p->rot_vy * (r_rot-1) / r_rot ;
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( Work *work, FMATRIX *m, MOVE *p, DG_MDL *mdl, int r_rot, float r_vel )
{
    int r, n, flag ;
    FVECTOR size = { BRK_DUST_SPHERE, 3.0f, BRK_DUST_SPHERE, 0 } ;

    n = !(p->rot_x/1024) ? 0 : p->rot_x>0 ? 2048 : -2048 ;
    r = ( n - p->rot_x ) / 16 ;
    r = p->rot_x + (p->rot_vx += r) ;
    if ( r == p->rot_x )
        flag = 1 ;
    else if ( (r > 0 && p->rot_x <= 0) || (r < 0 && p->rot_x >= 0) )
        r =    0, flag = 1 ;
    else if ( (!(p->rot_x/2048) && r/2048) || (p->rot_x/2048 && !(r/2048)) )
        r = 2048, flag = 1 ;
    else
        flag = 0 ;
    p->rot_x = r ;
    p->rot_y += p->rot_vy = (short)(p->rot_vy * (r_vel-1) / r_vel) ;

    p->pos_v.vy = -30.0f ;
    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( mdl )
	BRK_UTL_SizeOfMDL( mdl, m, &size ) ;
    if ( !BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v,
			   &BRK_HZD_NoBounce, &size ) )
    {
	p->pos_v.vy = 0 ;
        flag = -1 ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/***

  破片アクト関数

  ***/
/* 小さい破片のアクト起動関数 */
void BRK_PLT_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop )
{
    PART  *p ;

    work->n_piece &= ~BRK_PLT_INACTIVE ;
    while( --i>=0 )
    {
	FVECTOR r = { frnd() * width, rnd() * width + pop, frnd() * width, 0 } ;

	p = &work->piece[work->n_piece] ;
	p->flag = 1 ;

	p->mov.rot_vx = (irnd() & 63) - 32 ;
	p->mov.rot_vy = (irnd() & 511) - 256 ;
	p->mov.rot_x = 0 ;
	p->mov.rot_y = (irnd() & 4095) ;
	_sceVu0ScaleVector( &p->mov.pos_v, pos_v, 2.0f ) ;
	_sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
	_sceVu0ScaleVector( &r, &r, 2.5f ) ;
	_sceVu0AddVector( &p->mov.pos, pos, &r ) ;

	MakeMoveMatrix( &p->pos->world, &p->mov ) ;
	//BRK_UTL_ComdlColor( p->pos, 0.8f, work->where ) ;
	BRK_UTL_ComdlColor2( p->pos, Y ) ;

	if ( ++work->n_piece >= BRK_PLT_N_PIECE )
	    work->n_piece = 0 ;
    }    
}

/* 皿の破片のアクト関数 */
void BRK_PLT_ActPart( Work *work, int i, PART *p, int m_flag )
{
    int flag = 0, move_flg ;

    for ( ; --i>=0 ; flag|=p->flag, p++ )
	switch ( p->flag )
	{
	case 1:
	    move_flg = MoveObject( work, &p->pos->world, &p->mov,
				   p->mdl, BRK_ROT_R, BRK_VEL_R ) ;
	    if ( i&1 && m_flag && move_flg > 0 )
	    {
		BRK_PLT_StartActPiece( work, 5, &p->mov.pos, &DG_ZeroVector, 30.0f, 10.0f ) ;
		p->flag = 0 ;
		p->pos->color.vx = p->pos->color.vy = p->pos->color.vz = p->pos->color.vw = 0  ;
		if ( BRK_PLT_SeState2 <= 0 )
		{
		    GM_SeSetMode( (BRK_PLT_SeCount & 1 ? SD_A_SARAOTI1 : SD_A_SARAOTI2),
				  &p->mov.pos, GM_SEMODE_BOMB ) ;
		    BRK_PLT_SeCount++ ;
		    BRK_PLT_SeState2 = BRK_PLT_SE2_DFLT ;
		}
	    }
	    else
	    {
		BRK_UTL_ComdlColor2( p->pos, Y ) ;
		//BRK_UTL_ComdlColor( p->pos, 0.8f, work->where ) ;
		if ( move_flg == 1 )
		{
		    p->mov.rot_x &= 4095 ;
		    p->mov.rot_x = p->mov.rot_x>2048 ? p->mov.rot_x-4096 : p->mov.rot_x ;
		    p->flag = 2 ;
		}
	    }
	    break ;
	case 2:
	    move_flg = FallDownObject( work, &p->pos->world, &p->mov, p->mdl, BRK_ROT_R, BRK_VEL_R ) ;
	    p->flag = move_flg == 1 ? 0 :
		      move_flg ==-1 ? 1 : 2 ;
	    BRK_UTL_ComdlColor2( p->pos, Y ) ;
	    //BRK_UTL_ComdlColor( p->pos, 0.8f, work->where ) ;
	}
    if ( !flag )
    {
	if ( m_flag )
	    work->n_part  |= BRK_PLT_INACTIVE ;
	else
	    work->n_piece |= BRK_PLT_INACTIVE ;
    }
}


/* 皿の破片のアクト起動関数 起動は,皿1枚分の破片の数 */
void BRK_PLT_StartActPart( Work *work, PLATE *plt, FVECTOR *pos_v, float width, float pop )
{
    int     i ;
    PART   *p ;
    DG_MDL *m ;
    FVECTOR v ;

    work->n_part &= ~BRK_PLT_INACTIVE ;
    p = plt->parts ;
    m = work->p_def->models ;
    for ( i=work->p_def->n_models ; --i>=0 ; p++, m++ )
    {
	FVECTOR r = { frnd() * width, rnd() * width + pop, frnd() * width, 0 } ;

	v.vx = m->tx, v.vy = m->ty, v.vz = m->tz, v.vw = 0.0f ;

        p->flag = 1 ;
	p->mov = plt->mov ;
        p->mov.rot_x = p->mov.rot_y = 0 ;
        p->mov.rot_vx = (irnd() & 511) - 256 ;
        p->mov.rot_vy = (irnd() & 511) - 256 ;
        ApplyMatrixXYZ( &v, &plt->objs->world, &v ) ;
        _sceVu0AddVector( &p->mov.pos, &p->mov.pos, &v ) ;
        _sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;

	MakeMoveMatrix( &p->pos->world, &p->mov ) ;
	//BRK_UTL_ComdlColor( p->pos, 0.8f, work->where ) ;
	BRK_UTL_ComdlColor2( p->pos, Y ) ;
    }
    BRK_PLT_StartActPiece( work, BRK_PLT_N_BRKPIECE, &plt->mov.pos, pos_v, 30.0f, 20.0f ) ;
}


/* 皿のアクト関数 */
void BRK_PLT_ActPlate( Work *work, int i, PLATE *p )
{
    int flag = DG_FLAG_INVISIBLE ;

    for ( ; --i>=0 ; flag&=p->objs->flag, p++ )
        if ( !(p->objs->flag & DG_FLAG_INVISIBLE) )
	{
            if ( MoveObject( work, &p->objs->world, &p->mov, p->objs->objs->model,
			     BRK_ROT_R, BRK_VEL_R ) )
	    {
		/* 皿が落ちたので更に割れる */
		BRK_PLT_StartActPart( work, p, &DG_ZeroVector, 30.0f, 30.0f ) ;
		BRK_PLT_StartActDust( work, 5, &p->mov.pos, &DG_ZeroVector, 50.0f, 10.0f ) ;
                DG_InvisibleObjs( p->objs ) ;
		/* 皿が落ちて割れた音 */
		if ( BRK_PLT_SeState2 <= 0 )
		{
		    GM_SeSetMode( (BRK_PLT_SeCount & 1 ? SD_A_SARAOTI1 : SD_A_SARAOTI2),
				  &p->mov.pos, GM_SEMODE_BOMB ) ;
		    BRK_PLT_SeCount++ ;
		    BRK_PLT_SeState2 = BRK_PLT_SE2_DFLT ;
		}
	    }
	    //BRK_UTL_GetLightMatrix( &p->mov.pos, p->lights, 1.0f, work->where ) ;
	    DG_GetLightMatrix( &p->mov.pos, p->lights ) ;
	}
    if ( flag==DG_FLAG_INVISIBLE )
	work->n_plate |= BRK_PLT_INACTIVE ;
}

/* 皿のアクト起動関数 起動は1枚ずつ */
PLATE * BRK_PLT_StartActPlate( Work *work, FVECTOR *pos, FVECTOR *pos_v, float width, float pop )
{
    PLATE  *p ;
    FVECTOR r = { frnd() * width, rnd() * width + pop, frnd() * width, 0 } ;

    work->n_plate &= ~BRK_PLT_INACTIVE ;

    p = &work->plate[work->n_plate] ;
    p->mov.rot_vx = (irnd() & 127) - 64 ;
    p->mov.rot_vy = (irnd() & 127) - 64 ;
    p->mov.rot_x = p->mov.rot_y = 0 ;
    DG_VisibleObjs( p->objs ) ;
    _sceVu0CopyVector( &p->mov.pos, pos ) ;
    _sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
    MakeMoveMatrix( &p->objs->world, &p->mov ) ;

    //BRK_UTL_GetLightMatrix( &p->mov.pos, p->lights, 1.0f, work->where ) ;
    DG_GetLightMatrix( &p->mov.pos, p->lights ) ;

    if ( ++work->n_plate >= /*work->n_piled*/2*BRK_PLT_N_PLATE )
	work->n_plate = 0 ;
    return p ;
}

/* 積まれた皿のアクト関数 */
void BRK_PLT_ActPiled( Work *work, int i, PILED *p )
{
    PILED *pp ;
    float low ;
    int flag = 0 ;

    for ( ; --i>=0 ; flag|=p->flag, p++ )
    {
	DG_GetLightMatrix( (FVECTOR *)&p->objs->world.m[W], p->lights ) ;

	//BRK_UTL_GetLightMatrix( (FVECTOR *)&p->objs->world.m[W], p->lights, 1.0f, work->where ) ;
        if ( p->objs && p->flag & BRK_F_IS_ACTIVE )
	{
	    for ( pp=p-1 ; !(pp->flag & BRK_F_IS_PARENT) && (pp->flag & BRK_F_IS_BROKEN) ; pp-- );
	    low = pp->objs->world.m[W][Y] ;
	    if ( !(pp->flag & BRK_F_IS_BROKEN)  )
		  low += pp->target.size.vy * 2 ;
		p->vel_y -= BRK_GRAVITY ;
		p->objs->world.m[W][Y] += p->vel_y ;
	    if ( p->objs->world.m[W][Y] < low - 0.1f )
	    {
			p->vel_y = 0.0f ;
			p->flag &= ~BRK_F_IS_ACTIVE ;
			p->objs->world.m[W][Y] = low ;
	    }
	    GM_MoveTargetMap( &p->target, (FVECTOR *)&p->objs->world.m[W], work->where ) ;
	}
    }
    if ( !(flag & BRK_F_IS_ACTIVE) )
	work->n_piled |= BRK_PLT_INACTIVE ;
}



/***

  埃 アクト関数

  ***/
/* 埃のアクト関数 */
void BRK_PLT_ActDust( Work *work, int i )
{
    DG_PRIM2         *p = work->dust   ;
    FVECTOR          *v = work->dust_v ;
    FVECTOR          *nxt_p, *prv_p ;
    DG_PRIM2_UVRGBWH *nxt_u, *prv_u ;
    static FVECTOR DustGravity = { 0.0f, -3.0f, 0.0f } ;
    int flag = 0 ;

    prv_p = p->pos[p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos[p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;
    for ( ; --i>=0 ; flag|=nxt_u->a, nxt_u++, nxt_p++, prv_u++, prv_p++, v++ )
	if ( (nxt_u->a = prv_u->a) )
	{
	    nxt_u->a-- ;
	    if ( (v->vw -= 1.0f) > 0.0f )
		nxt_u->w = nxt_u->h = prv_u->h += (short)v->vw ;/*ある程度まで拡大*/
	    AddVector( v, v, &DustGravity ) ;
	    ScaleVector( v, v, 0.85f ) ;
	    AddVector( nxt_p, prv_p, v ) ;
	}

    if ( !flag )
	work->n_dust |= BRK_PLT_INACTIVE ;
}

/* 埃のアクト起動関数 */
void BRK_PLT_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v,
			   float width, float pop )
{
    DG_PRIM2         *p = work->dust ;
    FVECTOR          *v = work->dust_v ;
    FVECTOR          *nxt_p = p->pos  [p->buffer_clock] ;
    DG_PRIM2_UVRGBWH *nxt_u = p->uvrgb[p->buffer_clock] ;

    work->n_dust &= ~BRK_PLT_INACTIVE ;

    nxt_p += work->n_dust ;
    nxt_u += work->n_dust ;
    v     += work->n_dust ;
    while( --i>=0 )
    {
	FVECTOR r = { frnd() * width, rnd() * width + pop, frnd() * width, 0 } ;

        _sceVu0AddVector( v, pos_v, &r ) ;
        _sceVu0ScaleVector( &r, &r, 1.5f ) ;
        _sceVu0AddVector( nxt_p, pos, &r ) ;
	nxt_u->a = 32 ;
	nxt_u->w = nxt_u->h = 200 ;
	v->vw = 20.0f ;
	if ( ++work->n_dust >= BRK_PLT_N_DUST )
	{
	    work->n_dust = 0 ;
	    nxt_u -= BRK_PLT_N_DUST-1 ;
	    nxt_p -= BRK_PLT_N_DUST-1 ;
	    v     -= BRK_PLT_N_DUST-1 ;
	}
	else
	    nxt_u++, nxt_p++, v++ ;
    }
}

