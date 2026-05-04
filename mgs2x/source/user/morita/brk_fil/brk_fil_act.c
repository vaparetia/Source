//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_fil_act.c
   紙舞い壊れ

   2000/04/25 T. Morita
   $Id: brk_fil_act.c,v 1.1.1.3 2002/11/19 11:45:28 Yoshizawa1 Exp $
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

#include "../brk_utl/brk_utl.x"
#include "brk_file.h"



static FVECTOR BRK_FIL_NoBounce = { 1.01f, 1.0f, 1.01f, 0.0f } ;

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( Work *work, FMATRIX *m, MOVE *p, DG_MDL *mdl,
		       int r_rot, float r_vel, int stable )
{
    int     i, r, flag = 0 ;
    FVECTOR size = { BRK_DUST_SPHERE, 5.0f, BRK_DUST_SPHERE, 0 } ;
    FVECTOR v ;

    if ( stable )
    {
	r = p->rot_vx-1024 + p->rot_x ;
	for ( i=8 ; r/2 && --i>=0 ; )
	    r /= 2 ;
	p->rot_vx -= r ;
    }

    _sceVu0ScaleVector( &v, m->m[Y], ((p->rot_vx+1024)&2047)/32.0f ) ;
    if ( p->pos_v.vy < 0.0f ) /* 下向きに降りる時にこっち */
	v.vy = v.vy > 0.0f ? -v.vy : v.vy ;
    _sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.5f ) ;
    p->pos_v.vy -= BRK_GRAVITY ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, r_vel ) ;

    p->rot_x += p->rot_vx ;
    p->rot_y += p->rot_vy ;
    RotateMatrixXY( m, &DG_UnitMatrix,p->rot_x, p->rot_y ) ;
    if ( mdl )
        BRK_UTL_SizeOfMDL( mdl, m, &size ) ;
    switch( i=BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &BRK_FIL_NoBounce, &size ) )
    {
    case 3:
    case 1:
	flag = 1 ;
	if ( mdl )
	    if ( BRK_UTL_PutCenterHazard( work->hzd, &p->pos, &p->pos_v, 1000.0f, 25.0f ) )
		_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ), flag = 0 ;
	break ;
    case 2:
	p->pos.vy += p->pos_v.vy ;
	break ;
    case 0:
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( Work *work, FMATRIX *m, MOVE *p, DG_MDL *mdl, int r_rot, float r_vel )
{
    int     i, r, flag ;
    FVECTOR size = { BRK_DUST_SPHERE, 5.0f, BRK_DUST_SPHERE, 0 } ;

    //r =  (p->rot_x>0 ? 1024 : -1024) - p->rot_x ;
    r =  1024 - p->rot_x ;
    for ( i=4 ; r/2 && --i>=0 ; )
	r /= 2 ;
    r = p->rot_x + r ;
    if ( r == p->rot_x )
        flag = 1 ;
    else if ( (r/1024 && !(p->rot_x/1024)) || (!(r/1024) && p->rot_x/1024) )
        r = (p->rot_x>0 ? 1024 : -1024), flag = 1 ;
    else
        flag = 0 ;
    p->rot_x = r ;
    p->rot_y += p->rot_vy = (short)(p->rot_vy * (r_vel-1) / r_vel) ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.8f ) ;
    p->pos_v.vy = -BRK_GRAVITY*5 ;

    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( mdl )
        BRK_UTL_SizeOfMDL( mdl, m, &size ) ;
    if ( !(BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) & 1) )
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ), flag = 0 ;
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/***

  破片アクト関数

  ***/
/* 小さい破片のアクト起動関数 */
void BRK_FIL_ActPiece( Work *work )
{
    int    i, flag = 0 ;
    PIECE *p ;

    for ( i=BRK_FIL_N_PIECE, p=work->piece ; --i>=0 ; p++ )
    {
        switch ( p->flag )
        {
        case 1:
            if ( MoveObject( work, &p->pos->world, &p->mov, NULL, BRK_ROT_R, BRK_VEL_R, 1 ) == 1 )
	    {
		p->mov.rot_x &= 4095 ;
		p->mov.rot_x = p->mov.rot_x>2048 ? p->mov.rot_x-4096 : p->mov.rot_x ;
		p->mov.rot_y &= 4095 ;
		p->mov.rot_y = p->mov.rot_y>2048 ? p->mov.rot_y-4096 : p->mov.rot_y ;
		p->flag = 2 ;
	    }
            break ;
        case 2:
            if ( FallDownObject( work, &p->pos->world, &p->mov, NULL, BRK_ROT_R, BRK_VEL_R ) )
                p->flag = 0 ;
        }
	flag += p->flag ;
	BRK_UTL_ComdlColor( p->pos, 0.4f, work->where ) ;
    }

    if ( !flag )
	work->n_piece |= BRK_FIL_INACTIVE ;
}


void BRK_FIL_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop )
{
    PIECE  *p ;
    FVECTOR r ;

    work->n_piece &= ~BRK_FIL_INACTIVE ;
    for ( p=&work->piece[work->n_piece] ; --i>=0 ; p++ )
    {
        r.vx = width*frnd() ;
        r.vy = width* rnd() + pop ;
        r.vz = width*frnd() ;
        p->mov.rot_x  = irnd()&4095 ;
        p->mov.rot_y  = irnd()&4095 ;
        p->mov.rot_vx = (irnd()&255)-128 ;
        p->mov.rot_vy = (irnd()& 63)- 32 ;
        p->flag = 1 ;
        _sceVu0AddVector( &p->mov.pos  , pos  , &r ) ;
        _sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
        if ( ++work->n_piece >= BRK_FIL_N_PIECE )
            work->n_piece = 0, p -= BRK_FIL_N_PIECE ;
    }
}


/***

  紙アクト関数

  ***/
void BRK_FIL_ActFile( Work *work, PILED *piled )
{
    FILES *p = piled->file ;
    int    i ;
    int    flag =0 ;

    for ( i=BRK_FIL_N_PAPER ; --i>=0 ; p++ )
	switch( p->flag )
        {
        case 1:
            if ( MoveObject( work,
			     &p->objs->world, &p->mov,
			     p->objs->objs->model,
			     BRK_ROT_R, BRK_VEL_R, p->tic++>30 ) == 1 )
	    {
		p->mov.rot_x &= 4095 ;
		p->mov.rot_x = p->mov.rot_x>2048 ? p->mov.rot_x-4096 : p->mov.rot_x ;
		p->mov.rot_y &= 4095 ;
		p->mov.rot_y = p->mov.rot_y>2048 ? p->mov.rot_y-4096 : p->mov.rot_y ;
		p->flag = 2 ;
	    }
	    flag = 1 ;
	    /* ライトマトリックスを取得する */
	    DG_GetLightMatrix( &p->mov.pos, p->lights ) ;
	    //BRK_UTL_GetLightMatrix( &p->mov.pos, p->lights, 1.2f, work->where ) ;
            break ;

        case 2:
            switch( FallDownObject( work,
				    &p->objs->world, &p->mov,
				    p->objs->objs->model,
				    BRK_ROT_R, BRK_VEL_R ) )
	    {
	    case 1:
		p->flag = 0 ;
		break ;
	    case 2:
                p->flag = 1 ;
	    }
	    flag = 1 ;
	    /* ライトマトリックスを取得する */
	    BRK_UTL_GetLightMatrix( &p->mov.pos, p->lights, 1.2f, work->where ) ;
        }
    if ( !flag )
	piled->flag |= BRK_FIL_INACTIVE ;
}

int BRK_FIL_StartActFile( PILED *piled, FVECTOR *pos, FVECTOR *pos_v,
			  float width, float pop )
{
    FILES  *p = piled->file ;
    int     i ;
    FVECTOR r ;
    DG_DEF *def ;

    piled->work->n_piled &= ~BRK_FIL_INACTIVE ;
    piled->flag          &= ~BRK_FIL_INACTIVE ;
    for( i=BRK_FIL_N_PAPER ; --i>=0 ; p++ )
    {
	def = i&1 ? piled->work->m1_def : piled->work->m2_def ;
	def = piled->work->m1_def ;
	p->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
	if ( !p->objs )
	    PERROR( "Can't make DG_OBJS(No memory) :: NewPutFileObject\n" ) ;
	DG_SetLightMatrix( p->objs, p->lights ) ;
	DG_QueueObjs( p->objs ) ;
	GM_GroupObjs( p->objs, piled->work->where ) ;
	p->objs->objs[1].flag |= DG_FLAG_INVISIBLE ;

        r.vx = width*frnd() ;
        r.vy = width* rnd() + pop ;/* 上半球にするため rnd() を使う */
        r.vz = width*frnd() ;
        p->mov.rot_x  = 1024 ;
        p->mov.rot_vx = (irnd()& 63)- 32 ;
        p->mov.rot_y  = (irnd()&511)-256 ;
        p->mov.rot_vy = (irnd()& 63)- 32 ;
        p->flag = 1 ;
        _sceVu0CopyVector( &p->mov.pos, pos ) ;
        _sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
	p->mov.pos.vy += 100.0f ;
	p->tic = 0 ;

	RotateMatrixXY( &p->objs->world, &DG_UnitMatrix,
			p->mov.rot_x, p->mov.rot_y ) ;
	TransMatrix( &p->objs->world, &p->mov.pos ) ;
	BRK_UTL_GetLightMatrix( &p->mov.pos, p->lights, 0.8f,
				piled->work->where ) ;
    }    
    return  0 ;
}


/***

  埃 アクト関数

  ***/
/* 埃のアクト関数 */
void BRK_FIL_ActDust( Work *work )
{
    DG_PRIM2         *p = work->dust   ;
    FVECTOR          *v = work->dust_v ;
    FVECTOR          *nxt_p, *prv_p ;
    DG_PRIM2_UVRGBWH *nxt_u, *prv_u ;
    static FVECTOR DustGravity = { 0.0f, -0.25f, 0.0f } ;
    int i, flag = 0 ;

    prv_p = p->pos[p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos[p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;
    for ( i=BRK_FIL_N_DUST ; --i>=0 ; nxt_u++, nxt_p++, prv_u++, prv_p++, v++ )
	if ( (nxt_u->a = prv_u->a) )
	{
	    flag = 1 ;
	    if ( GV_Time & 1 )
		nxt_u->a-- ;
	    if ( (v->vw -= 2.0f) > 0.0f )
		nxt_u->w = nxt_u->h = prv_u->h += (short)v->vw ;/*ある程度まで拡大*/
	    AddVector( v, v, &DustGravity ) ;
	    ScaleVector( v, v, 0.85f ) ;
	    AddVector( nxt_p, prv_p, v ) ;
	}
    if ( !flag )
	work->n_dust |= BRK_FIL_INACTIVE ;
}

/* 埃のアクト起動関数 */
void BRK_FIL_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v,
			   float width, float pop )
{
    DG_PRIM2         *p = work->dust ;
    FVECTOR          *v = work->dust_v ;
    FVECTOR          *nxt_p = p->pos  [p->buffer_clock] ;
    DG_PRIM2_UVRGBWH *nxt_u = p->uvrgb[p->buffer_clock] ;

    work->n_dust &= ~BRK_FIL_INACTIVE ;
    nxt_p += work->n_dust ;
    nxt_u += work->n_dust ;
    v     += work->n_dust ;
    while( --i>=0 )
    {
	FVECTOR r = { frnd() * width, rnd() * width + pop, frnd() * width, 0 } ;

        _sceVu0AddVector( v, pos_v, &r ) ;
        _sceVu0ScaleVector( &r, &r, 1.5f ) ;
        _sceVu0AddVector( nxt_p, pos, &r ) ;
	nxt_u->a = 64 ;
	nxt_u->w = nxt_u->h = 100 ;
	v->vw = 20.0f ;
	if ( ++work->n_dust >= BRK_FIL_N_DUST )
	{
	    work->n_dust = 0 ;
	    nxt_u -= BRK_FIL_N_DUST ;
	    nxt_p -= BRK_FIL_N_DUST ;
	    v     -= BRK_FIL_N_DUST ;
	}
	else
	    nxt_u++, nxt_p++, v++ ;
    }
}

/*シェードを計算させるため*/
void BRK_FIL_ActPiled( Work *work )
{
    int    flag = BRK_FIL_INACTIVE ;
    int    i ;
    PILED *p = work->piled ;

    for ( i=work->n_piled ; --i>=0 ; p++ )
    {
	if ( !p->flag )
	    BRK_FIL_ActFile( work, p ) ;
	flag &= p->flag ;

	if ( p->objs )
	    BRK_UTL_GetLightMatrix( (FVECTOR*)&p->objs->world.m[W], p->lights, 0.8f, work->where ) ;
    }
    work->n_piled |= flag ;
}
