//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_b_line.c
  デモ用銃弾

  2001/01/27 T.Morita
  $Id: efct_b_line.c,v 1.1.1.3 2002/11/19 11:46:09 Yoshizawa1 Exp $
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

#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define DEMO_BUL_N_TRAIL   40
#define DEMO_BUL_N_POINT   5
#define DEMO_BUL_ALPHA_SUB 4

typedef struct point_t
{
    FVECTOR   v    ;
    float     tens ;
    float     cont ;
    float     bias ;
    float     frmno ;
} SPN_POINT ;

typedef struct tan_vec_t
{
    SPN_POINT *src ;
    SPN_POINT *dst ;

    float   base ;/* ベースのフレーム値 */
    float   frms ;/* 次のフレーム */
#if 0
    FVECTOR a1 ;
    FVECTOR a2 ;
    FVECTOR a3 ;
#else
    float a1[XYZ] ;
    float a2[XYZ] ;
    float a3[XYZ] ;
#endif
} TAN_VEC ;

typedef struct work_t
{
    GV_ACT_EX actor  ;

    int       count ;
    DG_PRIM2 *trail  ;
    DG_PRIM2 *smoke  ;

    SPN_POINT points[DEMO_BUL_N_POINT+2] ;
} Work ;







static inline float CheckDistanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}



/*

  タンジェントベクトルの計算

*/
static void CalcTangentVector( TAN_VEC *t,   /* tan_vec buffer     */
			SPN_POINT   *src, /* current key frame  */
			SPN_POINT   *dst, /* next key frame     */
			int      frms /* number of frames   */ )
{
    float a_dd1, a_dd2, a_ds1, a_ds2  ;
    int idx ;

    a_ds1 = (1.0f - dst->tens)*(1.0f - dst->cont)*(1.0f + dst->bias)*0.5f ;
    a_ds2 = (1.0f - dst->tens)*(1.0f + dst->cont)*(1.0f - dst->bias)*0.5f ;
    a_dd1 = (1.0f - src->tens)*(1.0f + src->cont)*(1.0f + src->bias)*0.5f ;
    a_dd2 = (1.0f - src->tens)*(1.0f - src->cont)*(1.0f - src->bias)*0.5f ;

    if ( frms )
	t->base = dst->frmno - (int)(t->frms = (float)frms ) ;
    else
	t->frms = dst->frmno - (int)(t->base = src->frmno) ;

#ifndef PSX2
	if ( t->frms== 0.0f ) t->frms = 0.0001f ;
#endif
    for( idx=XYZ ; --idx>=0 ; )
    {
	float dd, ds ;
	dd = (a_dd1*( ((float *)& src   ->v)[idx] - ((float *)&(src-1)->v)[idx]) +
	      a_dd2*( ((float *)&(src+1)->v)[idx] - ((float *)& src   ->v)[idx]) ) ;
	ds = (a_ds1*( ((float *)& dst   ->v)[idx] - ((float *)&(dst-1)->v)[idx]) +
	      a_ds2*( ((float *)&(dst+1)->v)[idx] - ((float *)& dst   ->v)[idx]) ) ;

	t->a1[idx] =  dd  ;
	t->a2[idx] =  3.0f*(((float *)&dst->v)[idx] - ((float *)&src->v)[idx]) - 2.0f*dd - ds ;
	t->a3[idx] =(-2.0f*(((float *)&dst->v)[idx] - ((float *)&src->v)[idx]) +      dd + ds)/t->frms ;
    }
    t->dst = dst ;
    t->src = src ;
}

/*
  スプライン計算

  <idx>の指定によりベクトルからスカラーまで利用可能。
  タンジェントベクトル<t>よりフレーム番号<frm>の計算結果が<a>の配列に入る。
*/
static void CalcSplineVector( TAN_VEC *t, float frm, FVECTOR *p )
{
    float s1, s2, s3 ;
    float frms = t->frms ;

    s1 = frm - t->base ;
    s2 = s1*s1 ;
    s3 = s1*s1*s1 ;
    p->vx = ((t->a3[X]*s3 + s2*t->a2[X])/frms + s1*t->a1[X])/frms + t->src->v.vx ;
    p->vy = ((t->a3[Y]*s3 + s2*t->a2[Y])/frms + s1*t->a1[Y])/frms + t->src->v.vy ;
    p->vz = ((t->a3[Z]*s3 + s2*t->a2[Z])/frms + s1*t->a1[Z])/frms + t->src->v.vz ;
    p->vw = 1.0f ;

    if ( s1 >= t->frms )
	CalcTangentVector( t, t->dst, t->dst+1, 0 ) ;
}

/*

  タンジェントベクトルの初期化

*/
static void CalcSplineVectorPos( SPN_POINT *head, float frm, FVECTOR *p )
{
    TAN_VEC tan_vec ;

    if ( frm < 0.0f )
	frm = 0.001f ;
    while( head->frmno < frm )
	head++ ;
    CalcTangentVector( &tan_vec, head-1, head, 0 ) ;
    CalcSplineVector(  &tan_vec, frm, p ) ;
}


static void Act( Work *work )
{
    int       i ;
    DG_PRIM2 *p ;
    FVECTOR          *nxt_p ;
    DG_PRIM2_UVRGB   *nxt_u, *prv_u ;
    DG_PRIM2_UVRGBWH *nxt_us, *prv_us ;
    int flag = 0 ;

    /*軌跡の表示*/
    p = work->trail ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_u = p->uvrgb[p->buffer_clock] ;
    for ( i=0 ; i<DEMO_BUL_N_TRAIL ; i++ )
    {
	nxt_u->a = prv_u->a ;
	if ( work->count < 0 )
	{
	    if ( i < 45+work->count*3 )
		nxt_u->a = 64 ;
	}
	else if ( i < work->count )
	    if ( nxt_u->a )
		nxt_u->a -= nxt_u->a>4 ? 4 : nxt_u->a ;
	flag |= nxt_u->a ;
	nxt_u++ ;
	prv_u++ ;
    }

    if ( work->count >= 0 )
    {
	/*煙の表示*/
	p = work->smoke ;
	prv_us = p->uvrgb[p->buffer_clock] ;
	DG_SwitchBuffPrim2( p ) ;
	nxt_p  = p->pos  [p->buffer_clock] ;
	nxt_us = p->uvrgb[p->buffer_clock] ;
	for ( i=0 ; i<DEMO_BUL_N_TRAIL ; i++ )
	{
	    if ( i < work->count )
	    {
		if ( (nxt_us->a = prv_us->a) )
		{
		    float a, r ;
#define ANGtoRAD(_a) ((_a)*(float)M_PI/128.0f)
		    r = (work->count - i) *  5.0f ;
		    r = r>200.0f ? 200.0f : r ;
		    a = ANGtoRAD( GM_StagePlayTime & 0xff ) + i ;
		    nxt_us->w = (short)(r * vu0_Cos( a )) ;
		    nxt_us->h = (short)(r * vu0_Sin( a )) ;
		    if ( !((GM_StagePlayTime+i) & 0xf) )
			nxt_us->a-- ;

		    /* 曲線を作る */
		    r = (float)(work->count - i) ;
		    r = r > 16.0f ? 0.0f : 1.0f - r/16.0f ;
		    CalcSplineVectorPos( work->points, (float)i*0.5f-r, nxt_p ) ;
		    nxt_p->vy += (work->count - i)/3.0f ;
		}
	    }
	    flag |= nxt_us->a ;
	    nxt_us++ ;
	    prv_us++ ;
	    nxt_p++  ;
	}

	if ( !flag )
	    GV_DestroyActor( work ) ;
	work->count += 5 ;
    }
    work->count++ ;


    //{ int i; if (GV_Time&1) for( i=5; --i>=0 ; ) AN_Test_Eye2( &work->points[i+1].v, 2 ) ; }
}

static void Die( Work *work )
{
    if ( work->trail )
	GM_FreePrim2( work->trail ) ;
    if ( work->smoke )
	GM_FreePrim2( work->smoke ) ;
}

static int GetResourcesCall( Work *work, FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid )
{
    TAN_VEC tan_vec ;
    FVECTOR v, h, dir ;
    int     i   ;
    static SPN_POINT points_org[DEMO_BUL_N_POINT+2] = {
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f,  0.0f },/*dummy*/
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f,  0.0f },/* 0th */
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 1.0f, 10.0f },/* 1st */
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f, 20.0f },/* 2nd */
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f, 30.0f },/* 3rd */
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f, 50.0f },/* 4th */
	{ { 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f, 50.0f },/*dummy*/
    } ;

    if ( !(work->smoke = BRK_UTL_MakeRSPRTWH( DEMO_BUL_N_TRAIL,
					      GV_StrCode( "bombpowder1_msk" ),
					      SCE_GS_SET_ALPHA(0,2,0,1,0),
					      0.0f,
					      0x087f7f7f )) )
	PERROR( "Cannot create DG_PRIM2 (May be no memroy) :: NewFortBulletCalled\n" ) ;

    if ( !(work->trail = BRK_UTL_MakeLINE2( 1, DEMO_BUL_N_TRAIL, 0,
					   SCE_GS_SET_ALPHA(0,2,0,1,0),
					    0x004f7f7f,
					    0x004f7f7f )) )
	PERROR( "Cannot create DG_PRIM2 (May be no memroy) :: NewFortBulletCalled\n" ) ;


    /* 垂直方向を計算 */
    _sceVu0SubVector( &h, avoid, pos ) ;
    _sceVu0OuterProduct( &h, &h, pos_v ) ;
    _sceVu0OuterProduct( &h, &h, pos_v ) ;
    _sceVu0Normalize( &h, &h ) ;

    /* 方向をランダムに修正 */
    _sceVu0Normalize( &dir, pos_v ) ;
    _sceVu0ScaleVector( &v, &h, 0.5f*frnd() ) ;
    _sceVu0AddVector( &dir, &dir, &v ) ;


    /* ポイントを初期化 */
    for ( i=DEMO_BUL_N_POINT+2 ; --i>=0 ; )
	work->points[i] = points_org[i] ;

    /* 各ポイントを計算 */
    _sceVu0CopyVector( &work->points[0].v, pos ) ;
    _sceVu0CopyVector( &work->points[1].v, &work->points[0].v ) ;

    _sceVu0ScaleVector( &v, &dir, -1000.0f ) ;
    _sceVu0AddVector( &work->points[2].v, &v, avoid ) ;
    _sceVu0ScaleVector( &v, &dir, 100.0f ) ;
    work->points[2].v.vx -= v.vz ;
    work->points[2].v.vz += v.vx ;

    _sceVu0ScaleVector( &v, &h, 1000.0f * ( 1.0f + 0.2f*rnd() ) ) ;
    _sceVu0AddVector( &work->points[3].v, &v, avoid ) ;

    _sceVu0ScaleVector( &v, &dir, 2000.0f ) ;
    _sceVu0AddVector( &work->points[4].v, &v, &work->points[3].v ) ;
    _sceVu0ScaleVector( &v, &h, 400.0f ) ;
    _sceVu0AddVector( &work->points[4].v, &v, &work->points[4].v ) ;

    _sceVu0ScaleVector( &v, &dir, 2000.0f ) ;
    _sceVu0AddVector( &work->points[5].v, &v, &work->points[4].v ) ;
    _sceVu0ScaleVector( &v, &h, 400.0f ) ;
    _sceVu0AddVector( &work->points[5].v, &v, &work->points[5].v ) ;

    _sceVu0CopyVector( &work->points[6].v, &work->points[5].v ) ;

    /* タンジェントベクトルを初期化 */
    CalcTangentVector( &tan_vec, &work->points[1], &work->points[2], 0 ) ;
    /* 曲線を作る */
    for ( i=0 ; i<DEMO_BUL_N_TRAIL ; i++ )
    {
	CalcSplineVector( &tan_vec, (float)i, &work->trail->pos[0][i] ) ;
	_sceVu0CopyVector( &work->trail->pos[1][i], &work->trail->pos[0][i] ) ;
	_sceVu0CopyVector( &work->smoke->pos[0][i], &work->trail->pos[0][i] ) ;
	_sceVu0CopyVector( &work->smoke->pos[1][i], &work->trail->pos[0][i] ) ;
    }

    work->count = -5*3 ;

    return 0 ;
}

void *NewFortSplineBulletCalled( FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResourcesCall( work, pos, pos_v, avoid ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
