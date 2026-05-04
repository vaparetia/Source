//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_mgz_act.c
   雑誌壊れ アクト

   2000/06/20 T. Morita
   $Id: put_bok_act.c,v 1.1.1.3 2002/11/19 11:46:29 Yoshizawa1 Exp $
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

#include "put_book.h"



FVECTOR PUT_BOK_Size = { PUT_BOKER_RAD,PUT_BOKER_RAD,PUT_BOKER_RAD, 0 } ;

#if 0
/***

  アクト補助関数

  ***/
/* DG_GetLightMatrixを用い光源計算 */
static void CalcPartsColor( DG_COMDL_POS *p )
{
    FVECTOR c = { 0.5f, 0.5f, 0.5f, 1.0f } ;
    FMATRIX lgt[2] ;

    DG_GetLightMatrix( (FVECTOR *)&p->world.m[W], lgt ) ;
#if 0
    c.vx *= GV_VecLen3F( (FVECTOR*)&lgt[0].m[X] ) ;
    c.vy *= GV_VecLen3F( (FVECTOR*)&lgt[0].m[Y] ) ;
    c.vz *= GV_VecLen3F( (FVECTOR*)&lgt[0].m[Z] ) ;
    printf( "c%.2f %.2f %.2f\n", c.vx, c.vy, c.vz ) ;
#else
    c.vx = _sceVu0InnerProduct( (FVECTOR*)&lgt[0].m[X], p->world.m[Y] ) ;
    c.vy = _sceVu0InnerProduct( (FVECTOR*)&lgt[0].m[Y], p->world.m[Y] ) ;
    c.vz = _sceVu0InnerProduct( (FVECTOR*)&lgt[0].m[Z], p->world.m[Y] ) ;
    c.vx = c.vx<0.0f ? -c.vx : c.vx<0.2f ? 0.2f : c.vx ;
    c.vy = c.vy<0.0f ? -c.vy : c.vy<0.2f ? 0.2f : c.vy ;
    c.vz = c.vz<0.0f ? -c.vz : c.vz<0.2f ? 0.2f : c.vz ;
    //printf( "c%.2f %.2f %.2f\n", c.vx, c.vy, c.vz ) ;
#endif
    _sceVu0ApplyMatrix( &c, &lgt[1], &c ) ;
    p->color.vx = c.vx>148 ? 148 : c.vx ;
    p->color.vy = c.vy>148 ? 148 : c.vy ;
    p->color.vz = c.vz>148 ? 148 : c.vz ;
    p->color.vw = 128  ; 
}
#endif


float PUT_BOK_GetCurrentSize( BOOK *mgz, FMATRIX *m, FVECTOR *size )
{
    int     i, j ;
    FVECTOR f, min, max ;
    DG_OBJ *o = mgz->objs->objs ;

    _sceVu0CopyMatrix( &mgz->objs->objs[0].world, m ) ;
    PUT_BOK_DispBook( mgz ) ;

    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
    for( j=2 ; --j>=0 ; o++ )
	for ( i=8 ; --i>=0 ; )
	{
	    f.vx = i&1 ? o->model->ux : o->model->lx ;
	    f.vy = i&2 ? o->model->uy : o->model->ly ;
	    f.vz = i&4 ? o->model->uz : o->model->lz ;
	    ApplyMatrixXYZ( &f, &o->world, &f ) ;
	    MinMaxVector( &min, &max, &f ) ;
	}
    _sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;

    return -min.vy ;
}

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( Work *work, FMATRIX *m, MOVE *p, BOOK *mgz,
		       float gravity, int hzx_flag )
{
    int     i, r, flag = 0 ;
    FVECTOR size = { PUT_DUST_SPHERE, PUT_DUST_SPHERE, PUT_DUST_SPHERE, 0 } ;
    FVECTOR v ;
    float flr[2] ;

    /* rot.vxは,1024を目指して回転させる。rot.vx は,-+1024でなければならない */
    r = p->rot_v.vx+ 1024 + p->rot.vx ;
    for ( i=10 ; r/2 && --i>=0 ; )
	r /= 2 ;
    p->rot_v.vx -= r ;
    /* -2048<rot<2048 を満たすようにする */
    p->rot.vx = (p->rot.vx + p->rot_v.vx) & 4095 ;
    p->rot.vx = p->rot.vx>2048 ? p->rot.vx-4096 : p->rot.vx ;
    p->rot.vy = (p->rot.vy + p->rot_v.vy) & 4095 ;
    p->rot.vy = p->rot.vy>2048 ? p->rot.vy-4096 : p->rot.vy ;

    if ( gravity > 0.0f )
	p->pos_v.vy -= gravity ;
    else
	p->pos_v.vy += gravity ;

    //RotateMatrixZY( m, &DG_UnitMatrix, p->rot.vx, p->rot.vy ) ;
    RotateMatrix( m, &DG_UnitMatrix, &p->rot ) ;

    if ( mgz )
	/*雑誌は Targetよりモデルの一番最下点を求める*/
	PUT_BOK_GetCurrentSize( mgz, m, &size ) ;
    else
    {
	/*ゴミは現在の向きに進む速度を現在の速度に足して平均化し
	  ていく。これによりフワフワする */
	_sceVu0ScaleVector( &v,
			    (FVECTOR*)&m->m[X],
			    ((p->rot_v.vx+1024)&2047)/32.0f ) ;

	if ( p->pos_v.vy < 0.0f ) /*一度下向きになったら,もう上がらない*/
	    v.vy = v.vy > 0.0f ? -v.vy : v.vy ;
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.5f ) ;
    }

    hzx_flag = HZX_LevelHazardCheck( work->hzx,
				     &p->pos,
				     HZX_CHK_ALL,
				     HZX_FLOOR_PITFALL ) ;
    if ( hzx_flag )
	HZX_GetLevelHeight( flr ) ;
    if ( !(hzx_flag & 1) )
	flr[0] = -60000.0f ;
    if ( !(hzx_flag & 2) )
	flr[1] = flr[0] + 3000.0f ;

    flr[0] += size.vy ;/* 大きさを踏まえた床と天井にする */
    flr[1] -= size.vy ;

    if ( gravity > 0.0f && p->pos.vy + p->pos_v.vy < flr[0] )
	flag = 1 ;
    else if ( (p->pos.vy > flr[1] && p->pos_v.vy<=0.0f) ||
	      p->pos.vy + p->pos_v.vy < flr[1] )
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( Work *work, FMATRIX *m, MOVE *p, BOOK *mgz, float gravity )
{
    int     r, flag ;
    FVECTOR size = { PUT_DUST_SPHERE, PUT_DUST_SPHERE, PUT_DUST_SPHERE, 0 } ;
    float flr[2] ;

    /* -2048<rot<2048 を満たすようにする */
#if 0
    r = p->rot.vx + (p->rot_v.vx += (p->rot.vx>0 ? 10 : -10) * (p->rot.vx/1024 ? -1 : 1) ) ;
    if ( (!(r/1024) && p->rot.vx/1024) || (r/1024 && !(p->rot.vx/1024)) )
        r = r>0 ? 1024 : -1024, flag = 1 ;
    else
        flag = 0 ;
#else
    if ( mgz )
    {
	 p->rot.vy    = (p->rot.vy + p->rot_v.vy) & 4095 ;
	 p->rot.vy   -= p->rot.vy  >2048 ? 4096 : 0 ;
	 p->rot_v.vy  = (mgz->rot.vy - p->rot.vy) & 4095 ;
	 p->rot_v.vy -= p->rot_v.vy>2048 ? 4096 : 0 ;
	 p->rot_v.vy /= 4 ;
    }

    flag = 0 ;
    p->rot_v.vx += (p->rot.vx/1024 ? -10 : 10) ;
    r = p->rot.vx + p->rot_v.vx ;
    if ( (!(r/1024) && p->rot.vx/1024) || (r/1024 && !(p->rot.vx/1024)) )
    {
        r = 1024 ;
	p->rot_v.vx = 0 ;
	if ( !p->rot_v.vy )
	  p->rot.vy = mgz->rot.vy, flag = 1 ;
    }
    
#endif
    p->rot.vx  = r & 4095 ;
    p->rot.vx -= p->rot.vx>2048 ? 4096 : 0 ;

    RotateMatrix( m, &DG_UnitMatrix, &p->rot ) ;
    if ( mgz )
        PUT_BOK_GetCurrentSize( mgz, m, &size ) ;

    if ( HZX_LevelHazardCheck( work->hzx, &p->pos,
			       HZX_CHK_ALL, HZX_FLOOR_PITFALL ) )
    {
	HZX_GetLevelHeight( flr ) ;
	p->pos.vy = flr[0] + size.vy ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/***

  破片アクト関数

  ***/
#if 0
/* 小さい破片のアクト起動関数 */
void PUT_BOK_ActPiece( Work *work )
{
    int    i, flag = 0 ;
    PIECE *p ;

    for ( i=PUT_BOK_N_PIECE, p=work->piece ; --i>=0 ; p++ )
    {
        switch ( p->flag )
        {
        case 1:
            if ( MoveObject( work, &p->pos->world, &p->mov,
			     NULL, PUT_GRAVITY, 1 ) == 1 )
		p->flag = 2 ;
            break ;
        case 2:
            if ( FallDownObject( work, &p->pos->world, &p->mov,
				 NULL, PUT_GRAVITY ) )
                p->flag = 0 ;
        }
	flag += p->flag ;
	CalcPartsColor( p->pos ) ;
    }
    if ( !flag )
	work->n_piece |= PUT_BOK_INACTIVE ;
}

void PUT_BOK_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop )
{
    PIECE  *p ;
    FVECTOR r ;

    work->n_piece &= ~PUT_BOK_INACTIVE ;
    for ( p=&work->piece[work->n_piece] ; --i>=0 ; p++ )
    {
        r.vx = width*frnd() ;
        r.vy = width* rnd() + pop ;
        r.vz = width*frnd() ;
        p->mov.rot.vx  = irnd()&4095 ;
        p->mov.rot.vy  = irnd()&4095 ;
        p->mov.rot_v.vx = (irnd()&255)-128 ;
        p->mov.rot_v.vy = (irnd()& 64)- 32 ;
        p->flag = 1 ;
        _sceVu0AddVector( &p->mov.pos  , pos  , &r ) ;
        _sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
        if ( ++work->n_piece >= PUT_BOK_N_PIECE )
            work->n_piece = 0, p -= PUT_BOK_N_PIECE ;
    }    
}
#endif


/***

  ページアクト関数

  ***/
/* ページのアクト関数 */
void PUT_BOK_ActPage( Work *work )
{
    int   i, flag = 0 ;
    float open ;
    PAGE *p ;

    for ( i=PUT_BOK_N_PAGE, p=work->page ; --i>=0 ; p++ )
    {
	if ( p->flag < -PUT_BOK_SPOOLTIME )
	    continue ;
	else if ( p->flag == -PUT_BOK_SPOOLTIME )
	{
	    PUT_BOK_FreePage( p ) ;
	}
        else if ( p->flag > 0 )
        {
	    if ( p->anime->count )
		SimpleVertexAnimation( p->anime ) ;

	    /* ページがめくり終るときは,平に戻す */
	    if ( p->flag == 5 )
	    {
		p->anime->p[0] = 1.0f, p->anime->p[1] = 0.0f ;
		p->anime->count = 1-(int)(DEG2RAD(15.0f)/p->book->open_v) ;
		p->anime->count = p->anime->count>4 ? 4 : p->anime->count ;
	    }
	    /* ページがめくり終ったのでページを消す */
	    else if ( p->flag == 1 )
		DG_InvisibleObjs( p->objs ) ;
	    else if ( p->flag > 5 )
	    {
		/* 本の開き具合によりページの歪みを制御する */
		if ( p->book->open                 >= DEG2RAD(30.0f) &&
		     p->book->open+p->book->open_v <  DEG2RAD(30.0f) )
		    p->anime->p[0] = 1.0f, p->anime->p[1] = 0.0f,
			p->anime->count = 1-(int)(DEG2RAD(15.0f)/p->book->open_v) ;
		else if ( p->book->open                 <  DEG2RAD(30.0f) &&
			  p->book->open+p->book->open_v >= DEG2RAD(30.0f) )
		    p->anime->p[0] = 0.0f, p->anime->p[1] = 1.0f,
			p->anime->count = 3 ;
	    }
	    p->open += p->open_v = (p->book->open - p->open)/(float)p->flag ;
	    open = p->book->open - DEG2RAD(10.0f) ;
	    p->open = p->open < DEG2RAD(5.0f) ? DEG2RAD(5.0f) :
		      p->open > open ? ( open>0.0f ? open : 0.0f ) :
		      p->open ;
	    _sceVu0RotMatrixY( &p->objs->world,
			       &DG_UnitMatrix,
			       p->open - p->book->open * 0.5f ) ;
	    _sceVu0MulMatrix( &p->objs->world, &p->book->objs->world, &p->objs->world ) ;
        }
	p->flag-- ;
	flag = 1 ;
    }
    if ( !flag )
	work->n_page |= PUT_BOK_INACTIVE ;
}

/* ページのアクト起動関数 */
void PUT_BOK_StartActPage( Work *work, BOOK *m, int frame )
{
    PAGE   *p = work->page ;
    int i ;

    for( i=PUT_BOK_N_PAGE, p=work->page ; p->flag>0 && --i>0 ; p++ ) ;

    if ( !p->objs )
	if ( (p->objs = DG_MakeObjs( work->n_def,
				     DG_FLAG_SHADE|DG_FLAG_ONEPIECE,
				     0 )) )
	{
	    if ( (p->anime = InitVertexAnimation( p->objs->objs,
						  work->c_def->models,
						  DG_VANIME_VERTS|DG_VANIME_NORMS,
						  work->c_def->n_models ) ) )
	    {
		DG_SetLightMatrix( p->objs, m->lights ) ;
		DG_QueueObjs( p->objs ) ;
		for ( i=work->c_def->n_models ; --i>=0 ; )
		{
		    p->anime->key[i] = &work->c_def->models[i] ;
		    p->anime->p[i] = 0.0f ;
		}
	    }
	    else
		DG_FreeObjs( p->objs ), p->objs = NULL ;
	}

    if ( p->objs )
    {
	work->n_page &= ~PUT_BOK_INACTIVE ;
	DG_VisibleObjs( p->objs ) ;
	p->open   = 0 ;
	p->open_v = m->open/frame ;
	p->book   = m ;
	p->flag   = frame ;
	p->anime->p[0] = 1.0f, p->anime->p[1] = 0.0f, p->anime->count = 0 ;
	SimpleVertexAnimation( p->anime ) ;
	if ( m->open >= DEG2RAD(30.0f) )
	    p->anime->p[0] = 0.0f, p->anime->p[1] = 1.0f, p->anime->count = 3 ;
    }
}


/***

  雑誌アクト関数

  ***/
//obj毎の回転
void PUT_BOK_DispBook( BOOK *m )
{
    DG_OBJ *obj   = m->objs->objs ;

    _sceVu0RotMatrixY( &obj[0].world, &DG_UnitMatrix, -m->open*0.5f ) ;
    _sceVu0MulMatrix ( &obj[0].world, &m->objs->world, &obj[0].world ) ;
	obj[0].world.m[W][Y] += m->y_offset ;

    _sceVu0RotMatrixY( &obj[1].world, &DG_UnitMatrix,  m->open*0.5f ) ;
    _sceVu0MulMatrix ( &obj[1].world, &m->objs->world, &obj[1].world ) ;
	obj[1].world.m[W][Y] += m->y_offset ;
}

void PUT_BOK_ActBook( Work *work )
{
    BOOK *p = &work->book ;

    switch ( p->flag )
    {
    default:
	 MoveObject( work, &p->objs->world, &p->mov, p, -PUT_BOK_GRAVITY, 0 ) ;
	 p->flag-- ;
	 p->se_tic = 6 ;
	 break ;

    case  3:
	/* 当たりつきで宙を舞う */
	if ( (MoveObject( work, &p->objs->world, &p->mov,
			  p, PUT_BOK_GRAVITY, 1 )) == 1 )
	{
	    GM_SeSetMode( SD_I_EROBFA01, &p->mov.pos, GM_SEMODE_BOMB ) ;
	    p->se_tic = 1 ;
	    p->flag = 2 ;
	}
	if ( (p->open += p->open_v) > DEG2RAD(30.0f) && !(GM_StagePlayTime & 7) )
	    PUT_BOK_StartActPage( work, p, 20 ) ;
	if ( p->open > ANG2RAD((1024-64)*2) )
	    p->open_v *= -0.8f, p->open = ANG2RAD((1024-64)*2) ;
	else if ( p->open < DEG2RAD(15.0f) )
	    p->open_v *= -0.8f, p->open = DEG2RAD( 15.0f) ;

	/* 無限落ち防止 */
	if ( (work->floor > GM_WaterLevel && 
	      p->mov.pos.vy < GM_WaterLevel-1000.0f) ||
	     (p->mov.pos.vy < -60000.0f) )
	    GV_CallParentSignalFunc( p->work, 1, 0 ) ;

	break ;

    case  2:
	if ( p->rot.vx/700 && p->se_tic )
	{
	    p->se_tic = 0 ;
	    GM_SeSetMode( SD_I_EROBFA01, &p->mov.pos, GM_SEMODE_BOMB ) ;
	}

	/* 地面についたので倒れる */
	if ( FallDownObject( work, &p->objs->world,
			     &p->mov, p, PUT_BOK_GRAVITY ) )
	{
	    p->flag = 1 ;
	    GV_CallParentSignalFunc( p->work, 2, 0 ) ;

	    if ( work->fall )
	    {
		/* 親に殺してもらう */
		GV_CallParentSignalFunc( p->work, 1, 0 ) ;
		work->blink = 1 ;
	    }
	}

    case  1:
	/*  開きになるようにする  */
	p->open += p->open_v = ( ANG2RAD((1024-64)*2) - p->open ) * 0.125f ;
	if ( !(int)(p->open_v/DEG2RAD(0.1f)) )
	    p->flag = p->flag==1 ? 0 : 2 ;
	break ;
    }
    PUT_BOK_DispBook( p ) ;
    DG_GetLightMatrix( &p->mov.pos, p->lights ) ;
    GM_MoveTarget2Map( &p->target[0], &p->objs->objs[0].world, work->where ) ;
    GM_MoveTarget2Map( &p->target[1], &p->objs->objs[1].world, work->where ) ;
}
