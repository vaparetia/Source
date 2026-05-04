/*
   brk_fed_act.c
   雑誌壊れ アクト

   2000/06/20 T. Morita
   $Id: brk_fed_act.c,v 1.1.1.3 2002/11/19 11:45:27 Yoshizawa1 Exp $
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

#include "brk_fedralex.h"
#include "../brk_utl/brk_utl.x"



FVECTOR BRK_FED_Size = { BRK_FEDER_RAD,BRK_FEDER_RAD,BRK_FEDER_RAD, 0 } ;

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

float BRK_MGS_GetCurrentSize( FEDRAL *fed, FMATRIX *m, FVECTOR *size, FVECTOR *cen )
{
    int     i ;
    FVECTOR f, min, max ;
    DG_OBJ *o = fed->objs->objs ;

    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
    for ( i=8 ; --i>=0 ; )
    {
	f.vx = i&1 ? o->model->ux : o->model->lx ;
	f.vy = i&2 ? o->model->uy : o->model->ly ;
	f.vz = i&4 ? o->model->uz : o->model->lz ;
	ApplyMatrixXYZ( &f, m, &f ) ;
	MinMaxVector( &min, &max, &f ) ;
    }
    _sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;
    _sceVu0AddVector( cen, &max, &min ) ;
    _sceVu0ScaleVector( cen, cen, 0.5f ) ;

    return -min.vy ;
}

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( Work *work, FMATRIX *m, MOVE *p, FEDRAL *fed, float gravity )
{
    int     i, r, flag = 0 ;
    FVECTOR size = { BRK_DUST_SPHERE, BRK_DUST_SPHERE, BRK_DUST_SPHERE, 0 } ;
    FVECTOR v ;

    /* rot_xは,1024を目指して回転させる。rot_x は,-+1024でなければならない */
    r = p->rot_vx+ 1024 + p->rot_x ;
    for ( i=10 ; r/2 && --i>=0 ; )
	r /= 2 ;
    p->rot_vx -= r ;
    /* -2048<rot<2048 を満たすようにする */
    p->rot_x = (p->rot_x + p->rot_vx) & 4095 ;
    p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
    p->rot_y = (p->rot_y + p->rot_vy) & 4095 ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;

    RotateMatrixZY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( fed )
	/*雑誌は Targetよりモデルの一番最下点を求める*/
        BRK_MGS_GetCurrentSize( fed, m, &size, &v ) ;
    else
    {
	/*ゴミは現在の向きに進む速度を現在の速度に足して平均化していく。これによりフワフワする */
	_sceVu0ScaleVector( &v, (FVECTOR*)&m->m[X], ((p->rot_vx+1024)&2047)/32.0f ) ;
	if ( p->pos_v.vy < 0.0f ) /* 一度下向きになったら,もう上がらない */
	    v.vy = v.vy > 0.0f ? -v.vy : v.vy ;
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.5f ) ;
    }

    p->pos_v.vy -= gravity ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;
    switch( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) )
    {
    case 2:
	p->pos.vy += p->pos_v.vy ;
	break ;
    case 1:
    case 3:
	flag = 1 ;
    case 0:
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

static int MoveObjectNonHit( Work *work, FMATRIX *m, MOVE *p, FEDRAL *fed, float gravity )
{
    int     i, r ;

    /* rot_xは,1024を目指して回転させる。rot_x は,-+1024でなければならない */
    r = p->rot_vx+ 1024 + p->rot_x ;
    for ( i=10 ; r/2 && --i>=0 ; )
	r /= 2 ;
    p->rot_vx -= r ;
    /* -2048<rot<2048 を満たすようにする */
    p->rot_x = (p->rot_x + p->rot_vx) & 4095 ;
    p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
    p->rot_y = (p->rot_y + p->rot_vy) & 4095 ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;

    RotateMatrixZY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    p->pos_v.vy -= gravity ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;
    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    TransMatrix( m, &p->pos ) ;

    return 0 ;
}


/* 0°or 180°に倒れ込み */
static int FallDownObject( Work *work, FMATRIX *m, MOVE *p, FEDRAL *fed, float gravity )
{
    int     r, flag ;
    FVECTOR cen ;
    FVECTOR size = { BRK_DUST_SPHERE, BRK_DUST_SPHERE, BRK_DUST_SPHERE, 0 } ;

    /* -2048<rot<2048 を満たすようにする */
    r = p->rot_x + (p->rot_vx += (p->rot_x>0 ? 10 : -10) * (p->rot_x/1024 ? -1 : 1) ) ;
    if ( (!(r/1024) && p->rot_x/1024) || (r/1024 && !(p->rot_x/1024)) )
        r = r>0 ? 1024 : -1024, flag = 1 ;
    else
        flag = 0 ;
    p->rot_x = r & 4095 ;
    p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
    //p->rot_vy = p->rot_vy * (BRK_VEL_R-1) / BRK_VEL_R ;
    //p->rot_y = (p->rot_y + p->rot_vy) & 4095 ;
    //p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;

    p->pos_v.vy = -300.0f ;
    RotateMatrixZY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( fed )
        BRK_MGS_GetCurrentSize( fed, m, &size, &cen ) ;
    if ( !BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) )
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/***

  破片アクト関数

  ***/
/* 小さい破片のアクト起動関数 */
void BRK_FED_ActPiece( Work *work )
{
    int    i, flag = 0 ;
    PIECE *p ;

    for ( i=BRK_FED_N_PIECE, p=work->piece ; --i>=0 ; p++ )
    {
        switch ( p->flag )
        {
        case 1:
            if ( MoveObject( work, &p->pos->world, &p->mov, NULL, BRK_GRAVITY ) == 1 )
		p->flag = 2 ;
            break ;
        case 2:
            if ( FallDownObject( work, &p->pos->world, &p->mov, NULL, BRK_GRAVITY ) )
                p->flag = 0 ;
        }
	flag += p->flag ;
	CalcPartsColor( p->pos ) ;
    }
    if ( !flag )
	work->n_piece |= BRK_FED_INACTIVE ;
}

void BRK_FED_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop )
{
    PIECE  *p ;
    FVECTOR r ;

    work->n_piece &= ~BRK_FED_INACTIVE ;
    for ( ; --i>=0 ; p++ )
    {
	p = &work->piece[work->n_piece] ;
        r.vx = width*frnd() ;
        r.vy = width* rnd() + pop ;
        r.vz = width*frnd() ;
        p->mov.rot_x  = irnd()&4095 ;
        p->mov.rot_y  = irnd()&4095 ;
        p->mov.rot_vx = (irnd()&255)-128 ;
        p->mov.rot_vy = (irnd()& 64)- 32 ;
        p->flag = 1 ;
        _sceVu0AddVector( &p->mov.pos  , pos  , &r ) ;
        _sceVu0AddVector( &p->mov.pos_v, pos_v, &r ) ;
        if ( ++work->n_piece >= BRK_FED_N_PIECE )
            work->n_piece = 0 ;
    }    
}

#if 0
/***

  ページアクト関数

  ***/
/* ページのアクト関数 */
void BRK_FED_ActPage( Work *work )
{
    int   i, flag = 0 ;
    float open ;
    PAGE *p ;

    for ( i=BRK_FED_N_PAGE, p=work->page ; --i>=0 ; p++ )
    {
	if ( p->flag < -BRK_FED_SPOOLTIME )
	    continue ;
	else if ( p->flag == -BRK_FED_SPOOLTIME )
	{
	    DG_DequeueObjs( p->objs ) ;
	    DG_FreeObjs( p->objs ) ;
	    p->objs  = NULL ;
	    ExitVertexAnimation( p->anime ) ;
	    p->anime = NULL ;
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
	    p->open = p->open < DEG2RAD(0.0f) ? DEG2RAD(0.0f) :
		      p->open > open ? ( open>0.0f ? open : 0.0f ) :
		      p->open ;
	    _sceVu0RotMatrixY( &p->objs->world, &DG_UnitMatrix, p->open ) ;
	    _sceVu0MulMatrix( &p->objs->world, &p->book->objs->world, &p->objs->world ) ;
        }
	p->flag-- ;
	flag = 1 ;
    }
    if ( !flag )
	work->n_page |= BRK_FED_INACTIVE ;
}

/* ページのアクト起動関数 */
void BRK_FED_StartActPage( Work *work, FEDRAL *m, int frame )
{
    PAGE   *p = work->page ;
    int i ;

    for( i=BRK_FED_N_PAGE, p=work->page ; p->flag>0 && --i>0 ; p++ ) ;

    if ( !p->objs  )
	if ( (p->objs = DG_MakeObjs( work->n_def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	{
	    if ( (p->anime = InitVertexAnimation( p->objs->objs,
						  work->c_def->models,
						  DG_VANIME_VERTS|DG_VANIME_NORMS,
						  work->c_def->n_models ) ) )
	    {
		DG_SetLightMatrix( p->objs, m->lights ) ;
		DG_QueueObjs( p->objs ) ;
		for ( i=work->c_def->n_models ; --i>=0 ; )
		    p->anime->key[i] = &work->c_def->models[i], p->anime->p[i] = 0.0f ;
	    }
	    else
		DG_FreeObjs( p->objs ), p->objs = NULL ;
	}
    if ( p->objs )
    {
	work->n_page &= ~BRK_FED_INACTIVE ;
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
#endif




/***

  雑誌アクト関数

  ***/
void BRK_FED_ActFedral( Work *work )
{
    FEDRAL *p = work->fedral ;
    int    i, flag =0 ;

    for ( i=work->n_fedral ; --i>=0 ; p++ )
    {
	switch ( p->flag )
        {
	case  0:
	    /* なにもしない */
	    continue ;

	case 4:
	    /* ゴミを出して宙を舞う */
	    //BRK_FED_StartActPiece( p->work, 5, &p->mov.pos, &pop, 50.0f, 40.0f ) ;

	default:
	    /* 当たりなしで宙を舞う */
	    p->flag-- ;
	    MoveObjectNonHit( work, &p->objs->world, &p->mov, p, BRK_FED_GRAVITY ) ;
            break ;

	case  3:
	    /* 当たりつきで宙を舞う */
            if ( MoveObject( work, &p->objs->world, &p->mov, p, BRK_FED_GRAVITY ) == 1 )
	    {
		/* 敵兵見つかり用 */
		if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
		{
		    GM_SetNoise( NOISE_S, &p->mov.pos, work->where ) ;
		    _sceVu0CopyVector( &work->ene_find.pos, &p->mov.pos ) ;
		    work->ene_find.type = EF_TYPE_LV2 ;
		}
		p->flag = 2 ;
	    }
            break ;

	case  2:
	    /* 地面についたので倒れる */
	    if ( FallDownObject( work, &p->objs->world, &p->mov, p, BRK_FED_GRAVITY ) )
	    {
                p->flag = 1 ;
		p->objs->world.m[3][1] += p->y_offset;
	    }

	case  1:
	    /*  90°以上なら開き,それ以下なら閉じる。仰向けなら補間し,俯せなら強制  */
                p->flag = p->flag==1 ? 0 : 2 ;		
	    break ;
        }
	BRK_UTL_GetLightMatrix( &p->mov.pos, p->lights, 0.6f, p->work->where ) ;
	GM_MoveTarget2Map( &p->target, &p->objs->world, p->work->where ) ;
	flag = 1 ;
    }

    if ( !flag )
	work->n_fedral |= BRK_FED_INACTIVE ;
}


/***

  埃 アクト関数

  ***/
#if 0
/* 埃のアクト関数 */
void BRK_FED_ActDust( Work *work )
{
    DG_PRIM2         *p = work->dust   ;
    FVECTOR          *v = work->dust_v ;
    FVECTOR          *nxt_p, *prv_p ;
    DG_PRIM2_UVRGBWH *nxt_u, *prv_u ;
    static FVECTOR DustGravity = { 0.0f, -1.0f, 0.0f } ;
    int i, flag = 0 ;

    if(!work->dust) return;
    prv_p = p->pos[p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos[p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;
    for ( i=BRK_FED_N_DUST ; --i>=0 ; nxt_u++, nxt_p++, prv_u++, prv_p++, v++ )
	if ( (nxt_u->a = prv_u->a) )
	{
	    flag = 1 ;
	    if ( GV_Time & 1 )
		nxt_u->a-- ;
	    if ( (v->vw -= 2.0f) > 0.0f ){
			//work->size[i] += v->vw;
		//nxt_u->w = nxt_u->h = prv_u->h += v->vw ;/*ある程度まで拡大*/
		}
//		nxt_u->w = (short)(work->size[i] * cosf(work->rad[i]));
//		nxt_u->h = (short)(work->size[i] * sinf(work->rad[i]));
	    AddVector( v, v, &DustGravity ) ;
	    ScaleVector( v, v, 0.85f ) ;
	    AddVector( nxt_p, prv_p, v ) ;
	}
    if ( !flag )
	work->n_dust |= BRK_FED_INACTIVE ;
}

/* 埃のアクト起動関数 */
void BRK_FED_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v,
			   float width, float pop )
{
    DG_PRIM2         *p = work->dust ;
    FVECTOR          *v = work->dust_v ;
    FVECTOR          *nxt_p = p->pos  [p->buffer_clock] ;
    DG_PRIM2_UVRGBWH *nxt_u = p->uvrgb[p->buffer_clock] ;

    work->n_dust &= ~BRK_FED_INACTIVE ;
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
//	work->rad[i] = M_PI * frnd();
//	work->size[i] = 100.0f;
//	nxt_u->w = (short)(100.0f  * cosf(work->rad[i]));
//	nxt_u->h = (short)(100.0f  * sinf(work->rad[i]));
	v->vw = 20.0f ;
	if ( ++work->n_dust >= BRK_FED_N_DUST )
	{
	    work->n_dust = 0 ;
	    nxt_u -= BRK_FED_N_DUST ;
	    nxt_p -= BRK_FED_N_DUST ;
	    v     -= BRK_FED_N_DUST ;
	}
	else
	    nxt_u++, nxt_p++, v++ ;
    }
}

#endif
