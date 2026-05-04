//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_rcg.c 
   オレガ 認知情報 関数群

   1999/12/22 T.Morita
   $Id: orga_rcg.c,v 1.1.1.3 2002/11/19 11:46:21 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "libutl.h"
#include "include/orga.h"
#include "libfs.h"




/*

  基本汎用関数

*/
static float CheckDistanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}

#if 0
static int CheckPointWithinTriangle( FVECTOR *vv, FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
    float a, b, c ;

    a = BackFaceCullingXZ( v0, v1, vv ) ;
    b = BackFaceCullingXZ( v1, v2, vv ) ;
    c = BackFaceCullingXZ( v2, v0, vv ) ;
    if ( (a>0 && b>0 && c>0) || (a<0 && b<0 && c<0) )/* vv は v0 v1 v2の3角形の中にあるか */
	return 1 ;
    return 0 ;
}
#endif

#if 1
static inline int CheckOnlineSegment( FVECTOR *p1, FVECTOR *p2, HZX_SEG *seg )
{
    float px, pz, sx, sz, tx, tz ;
    float rd, r, s ;

    px = p2->vx    - p1->vx    ; pz = p2->vz    - p1->vz    ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*pz - sz*px) == 0.0f )
	return 0 ;
    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*px - tx*pz)/rd ;
    if ( r > 1.0f || r < 0.0f )
	return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
	return 0 ;

    return 1 ;
}
#else
static inline int CheckOnlineSegment( FVECTOR *p1, FVECTOR *p2, HZX_SEG *seg )
{
    FVECTOR  p ;
    float sx, sz, tx, tz ;
    float rd, r, s ;

    _sceVu0SubVector( &p, p1, p2 ) ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*p.vz - sz*p.vx) == 0.0f )
	return 0 ;
    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*px - tx*pz)/rd ;
    if ( r > 1.0f || r < 0.0f )
	return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
	return 0 ;
    if ( seg.p1.y > p.vy*r && seg.p1.y+seg.p1.h < p.vy*r )
	return 0 ;
    return 1 ;
}
#endif

static inline int CheckOnlineTarget( FVECTOR *lpos, FVECTOR *ldir, TARGET *box )
{
    FVECTOR v   ;
    FMATRIX inv ;

    _sceVu0SubVector( &v, &box->center, lpos ) ;/*Coordinate to Box*/
    _sceVu0ScaleVector( &v, ldir, _sceVu0InnerProduct( ldir, &v ) ) ;
    _sceVu0AddVector( &v, &v, lpos ) ;  /*Nearest point to center of the Box*/
    _sceVu0SubVector( &v, &v, &box->center ) ;/*Coordinate to Box*/
    _sceVu0InversMatrix( &inv, &box->world ) ;
    _sceVu0ApplyMatrix( &v, &inv, &v ) ;
    _sceVu0SubVector( &v, &v, &box->offset ) ;

    /*DivVectorXYZ( &v, &v, &box->size ) ;*/
    /* 直線と ターゲットの当たり判定 */
    if ( (int)(v.vx/box->size.vx) || (int)(v.vy/box->size.vy) || (int)(v.vz/box->size.vz) )
	return 0 ;
    return 1 ;
}



/*

  ホロチェック関数

*/
int ORG_RecogCheckHoloValid( Work *work )
{
    return work->misc_holo_stat ? ( *work->misc_holo_stat == ORGA_HOL_ORG_FREE ||
				    *work->misc_holo_stat == ORGA_HOL_ORG_OPEN ) : 0 ;
}

int ORG_RecogCheckHoloCovered( Work *work )
{
    return work->misc_holo_stat ? ( *work->misc_holo_stat != ORGA_HOL_ORG_GONE &&
				    *work->misc_holo_stat != ORGA_HOL_PLY_GONL &&
				    *work->misc_holo_stat != ORGA_HOL_PLY_GONR ) : 0 ;
}

int ORG_RecogCheckHoloBlowValid( Work *work )
{
    return work->misc_holo_stat ? ( *work->misc_holo_stat == ORGA_HOL_PLY_FREL ||
				    *work->misc_holo_stat == ORGA_HOL_PLY_FRER ) : 0 ;
}

/*

  投光器チェック関数

*/
int ORG_RecogCheckSpotLight( Work *work )
{
    return work->misc_spot_stat ? *work->misc_spot_stat : 0 ;
}

/*

  ストリーム再生

*/
void ORG_RecogStartStream( Work *work )
{
    /* ハンドラを取得 */
	if ( work->str_id[work->voice_vox>>12] == 0 ) {
		return ;
	}
    work->str_hdl = GM_VoxStream( work->str_id[work->voice_vox>>12], GM_STREAM_FLAG_3D ) ;

    /*マルチウェイト表示*/
    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
    DG_InvisibleObjs( work->body.objs ) ;
}
void ORG_RecogStopStream( Work *work )
{
    /* ハンドラを初期化する（ストリームが使われていないことを意味する） */
    work->str_hdl = 0 ;

    /*シングルウェイト表示*/
    work->body.evmobj->flag |=  DG_EVMOBJ_INVISIBLE ;
    DG_VisibleObjs( work->body.objs ) ;
}


/*

  プレイヤーを狙うのは
  プレーヤの高さに合わせる。

*/
FVECTOR  *ORG_RecogAimPos( Work *work )
{
    if ( work->trgt_aim == &work->trgt_hid ||
	 (work->trgt_aim >= &ORG_DummyPos[0] && work->trgt_aim >= &ORG_DummyPos[14]) )
    {
	_sceVu0CopyVector( &work->trgt_aim_pos, work->trgt_aim ) ;
	if ( (ORGA_PLY_CAMERAPOS.vy - GM_PlayerControl->mov.vy > 550.0f) ||
	     (GM_CheckPlayerStatus( PLAYER_HOLD ) && ORG_Work->flag & ORGA_F_AIMED_NEAR) )
	    work->trgt_aim_pos.vy = ORGA_PLY_CAMERAPOS.vy ;
	else
	    work->trgt_aim_pos.vy = BODYPOS( GM_PlayerBody, HUMAN21_KOSHI )->vy ;

	return &work->trgt_aim_pos ;
    }
    return work->trgt_aim ;
}


/*

  プレイヤーの位置を正確に知るための関数
  一度計算したら,その結果を使い回す。

*/
FVECTOR  *ORG_RecogPlayerPosNoHide( int joint )
{
    if ( GM_CheckPlayerStatus( PLAYER_SQUAT ) )
    {
	/*覗き込みでプレーヤーの頭が出る*/
	/*主観で構えて近くを狙う        */
	if ( (ORGA_PLY_CAMERAPOS.vy - GM_PlayerControl->mov.vy > 550.0f) ||         
	     (GM_CheckPlayerStatus( PLAYER_HOLD ) && ORG_Work->flag & ORGA_F_AIMED_NEAR) )
	    return &ORGA_PLY_CAMERAPOS ;
	else
	    return BODYPOS( GM_PlayerBody, HUMAN21_KOSHI ) ;
    }
    if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
	return &ORGA_PLY_CAMERAPOS ;

    if ( joint == ORGA_GET_PLY_CONTROL )
	return &GM_PlayerControl->mov ;

    return BODYPOS( GM_PlayerBody, joint ) ;
}

FVECTOR  *ORG_RecogPlayerPos( int joint )
{
#if 0
    if ( ORG_Work->flag & ORGA_F_UNRECOG_POS && !CHECKACT( ORG_Work ) )
	return &ORG_Work->trgt_hid ;
#else
    if ( (ORG_Work->ply_hide & ORGA_F_PLYR_HIDING || ORG_Work->flag & ORGA_F_UNRECOG_POS) &&
	 !CHECKACT( ORG_Work ) )
	return &ORG_Work->trgt_hid ;
#endif

    return ORG_RecogPlayerPosNoHide( joint ) ;
}




/*

  相手が自分の体を狙っているかどうか（ターゲットレベル）

 */
static void ORG_RecogAimedBy( Work *work )
{
    FMATRIX *arm = &BODYWORLD( GM_PlayerArmBody, HUMAN21_MIGI_TE ) ;
    int     i ;

    work->flag &= ~ORGA_F_AIMED_NEAR ;
    if ( GM_CheckPlayerStatus( PLAYER_HOLD ) && GM_CheckPlayerStatus( PLAYER_WATCH ) )
	/*主観時にどれくらい近くを狙ってるか (軽いバージョン)*/
	if ( CheckDistanceLineAndPoint( (FVECTOR*)&arm->m[W], (FVECTOR*)&arm->m[Y],
					&work->control.mov ) < ORGA_ATTACK_RAD )
	{
	    work->flag |= ORGA_F_AIMED_NEAR ;
	    if (  work->flag & ORGA_F_ENB_AVOID &&
		!(work->flag & (ORGA_F_UNRECOG_POS|ORGA_F_UNRECOG_AIM)) )
	    {
		work->avoid_flg = 0 ;
		for ( i=ORGA_N_PARTS ; --i>=0 ; )
		    if ( CheckOnlineTarget( (FVECTOR*)&arm->m[W], (FVECTOR*)&arm->m[Y], 
					    &work->target[i+1] ) )
			work->avoid_flg |= (1 << i) ;
	    }
	}
}



/*

  相手の体の一部がいまの場所から見えるかどうか（セグメントに遮られているかどうか）

 */
#if 0
static inline int CheckVisibeParts( FVECTOR *p1, DG_MDL *m, DG_OBJ *o, HZX_SEG *seg )
{
    FVECTOR *p2, v[4] = {
	{ m->lx,m->ly,m->lz }, { m->ux,m->uy,m->uz }, { m->lx,m->uy,m->lz }, { m->ux,m->ly,m->uz }
    } ;
    int i ;

    /* p1位置から vの各四点が セグメントにより遮られているかどうかのチェック
       本来なら八点だけど,これで十分っしょ。*/
    for ( i=4, p2=v ; --i>=0 ; p2++ )
    {
	_sceVu0ApplyMatrix( p2, &o->world, p2 ) ;
	_sceVu0AddVector( p2, p2, (FVECTOR*)&o->world.m[3] ) ;

	if ( CheckOnlineSegment( p1, p2, seg ) )
	    return 1 ;
    }
    return 0 ;
}
#endif

static inline void ORG_RecogCanAim( Work *work )
{
    FVECTOR pos ;
    float  d ;
    int hzx_flag ;

    work->aim_flg = 0 ;
    hzx_flag = HZX_OnlineHazardCheck( work->control.hzx_id,
				      BODYPOS( &work->body, HUMAN21_MIGI_TE ),
				      ORG_RecogPlayerPos( HUMAN21_KUBI ),
				      HZX_CHK_F_SEGMENT/*| HZX_CHK_F_FLOOR*/,
				      /*HZX_SEG_NO_BULLET|*/ HZX_SEG_RECOIL_TYPE,
				      HZX_FLOOR_ALL ) ;
    if ( !hzx_flag )
	work->aim_flg = 1 ;
    else
    {
	HZX_GetOnlinePoint( &pos ) ;
	_sceVu0SubVector( &pos, &pos, BODYPOS(&work->body, HUMAN21_MIGI_TE) ) ;
	d = _sceVu0InnerProduct( &pos, &pos ) ;
	if ( d < 1500.0f*1500.0f )
	{
	    work->aim_flg = ORGA_F_DONT_SHOOT ;
	}
    }
    work->flag &= ~ORGA_F_ENB_AIM ;
}




/*

  安全地帯にいるのかどうか(廃止版,最新版はもう一つ下の関数)

*/
#if 0
static inline void ORG_RecogSafeZone( Work *work )
{
    HIDE *h = work->hide_spot;

    work->flag &= ~ORGA_F_HIDING_POS ;

    if ( CheckPointWithinTriangle( &work->control.mov, &h->p0, &h->p1, &h->p2 ) ||
	 CheckPointWithinTriangle( &work->control.mov, &h->p0, &h->p2, &h->p3 ) )
    {
	work->flag |= ORGA_F_HIDING_POS ;
	if ( h->height < ORGA_HEIGHT )
	    work->flag |= ORGA_F_DANGR_STAND ;
	if ( VectorSquare( &work->control.mov, &h->p0 ) < ORGA_HIDE_RADIUS )
	    work->flag |= ORGA_F_DANGR_RIGHT ;
	if ( VectorSquare( &work->control.mov, &h->p1 ) < ORGA_HIDE_RADIUS )
	    work->flag |= ORGA_F_DANGR_LEFT  ;
	work->exposed = 0 ;
    }
    else
	work->exposed++ ;
}
#endif


/*

  移動領域内の計算

*/
void ORG_CalcHidePos( FVECTOR *pos )
{
    float z ;

    if ( !pos )
	return ;
    z = ORGA_HIDE_V_SPHERE - 20000.0f ;
    if ( pos->vz < z )
	pos->vz = z ;
    z = -ORGA_HIDE_V_SPHERE - 12000.0f + 0.25f*(pos->vx + 12000.0f) ;
    if ( pos->vz > z )
	pos->vz = z ;
}

/*

  安全地帯の計算

*/
float ORG_CalcNewHidePosZ( float *p0, float *p1, FVECTOR *pp, float h, float x )
{
    FVECTOR a ;
    float z, z0, z1 ;

    /* 隠れ場所を計算 */
    a.vx = p0[X] - pp->vx ; a.vz = p0[Y] - pp->vz ; a.vy = a.vw = 0 ;
    _sceVu0Normalize( &a, &a ) ;
    z0 = ( a.vz * (x - pp->vx) - h ) / a.vx + pp->vz ;
    a.vx = p1[X] - pp->vx ; a.vz = p1[Y] - pp->vz ;
    _sceVu0Normalize( &a, &a ) ;
    z1 = ( a.vz * (x - pp->vx) - h ) / a.vx + pp->vz ;

    /* 動作可能領域の中か？ */
    z = (h>0 ? z0<z1 : z0>z1) ? z0 : z1 ;
    if ( z <  ORGA_HIDE_V_SPHERE - 20000.0f ||
	 z > -ORGA_HIDE_V_SPHERE - 12000.0f + 0.25f*(x+12000.0f) )
	z = 0 ;

    /* ビハインドだったら障害物からはみ出さない */
    else if ( !(int)((x-p1[X])/(ORGA_HIDE_B_SPHERE+1.0f)) )
	z = (h>0 ? p0[Y]>z : p0[Y]<z) ? p0[Y] : z ;

    return z ;
}

float ORG_GetHidePosZ( HIDE *hide, FVECTOR *pp, float h, int stll_mtn )
{
    float x ;

    x = hide->p[2][X] - ( stll_mtn&ORGA_F_IS_BEHIND ? ORGA_HIDE_B_SPHERE : ORGA_HIDE_V_SPHERE) ;
    if ( h > 0 )
	return ORG_CalcNewHidePosZ( hide->p[0], hide->p[2], pp,  ORGA_HIDE_H_SPHERE, x ) ;
    else
	return ORG_CalcNewHidePosZ( hide->p[1], hide->p[3], pp, -ORGA_HIDE_H_SPHERE, x ) ;
}

int ORG_RenewHidePosZ( HIDE *hide, FVECTOR *pp, int stll_mtn )
{
    float x ;
    int flag ;

    /* ビハインド用と普通ので分ける */
    x = hide->p[2][X] - ( stll_mtn&ORGA_F_IS_BEHIND ? ORGA_HIDE_B_SPHERE : ORGA_HIDE_V_SPHERE) ;
    hide->left.vx = hide->right.vx = x ;

    /* 隠れポジション Z座標を変更 */
    hide->left.vz  = ORG_CalcNewHidePosZ( hide->p[0], hide->p[2], pp,  ORGA_HIDE_H_SPHERE, x ) ;
    hide->right.vz = ORG_CalcNewHidePosZ( hide->p[1], hide->p[3], pp, -ORGA_HIDE_H_SPHERE, x ) ;
    flag  = hide->left.vz ==0.0f ? 0 : 1 ;
    flag |= hide->right.vz==0.0f ? 0 : 2 ;

    ORG_CalcHidePos( &hide->right ) ;
    ORG_CalcHidePos( &hide->left  ) ;

    return flag ;
}


/*

  ハイドの場所を再計算し,
  安全地帯にいるのかどうかをチェック

*/
static inline void ORG_RecogSafeZone( Work *work )
{
    FVECTOR *pos      ;
    float    z0, z1   ;
    int      watching ;

    if ( GM_CheckPlayerStatus( PLAYER_DAMAGED ) )
	watching = 0 ;
    else if ( work->flag & ORGA_F_UNRECOG_AIM )
	watching = -1 ;
    else 
    {
	pos = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
	z0 = ORG_GetHidePosZ( work->hide_spot, pos, ORGA_HIDE_H_SPHERE, work->stll_mtn ) ;
	if ( (z0==0.0f ? 1 : (work->trgt_pos->vz > z0-60.0f) ) )
	{
	    z1 = ORG_GetHidePosZ( work->hide_spot, pos, -ORGA_HIDE_H_SPHERE, work->stll_mtn ) ;
	    watching = ( z1==0.0f ? (z0==0.0f ? 1:0) : ((work->trgt_pos->vz < z1+60.0f) ? 0:1) ) ;
	}
	else
	    watching = 1 ;
    }

    if ( watching == 1 )
	work->exposed++   ;
    else if ( watching == 0 )
	work->exposed = 0 ;
    else if ( watching == -1 )
	if ( --work->exposed <= 0 )
	    work->exposed = 0 ;
}




/*

  プレイヤーの大まかな位置の監視

  0---1
  |   |
  3---2 の順番に並んでいる
*/
static inline void ORG_RecogHideStatus( Work *work )
{
    static const float (area[])[XY] = {
	{ -14000,-13750 }, { -14000,-14750 }, { -13500,-14750 }, { -13500,-13750 },/* 0 オルガ左前 */
       	{ -13500,-17600 }, { -13500,-18600 }, { -12500,-18600 }, { -12500,-17600 },/* 4 オルガ右前 */
       	{ -17600,-15450 }, { -17600,-18550 }, { -16400,-18550 }, { -16400,-15450 },/* 8 オルガ後ろ */

       	{ -11303,-11867 }, { -10932,-12903 }, { - 9897,-12532 }, { -10267,-11497 },/*12 左の上１つ */
       	{ -10890,-16542 }, { - 9808,-18954 }, { -10753,-18391 }, { - 9842,-16210 },/*16 右の上２つ */
       	{ -11850,-11450 }, { -11850,-12550 }, { - 9650,-12350 }, { - 9650,-11250 },/*20 左の下２つ */
       	{ -10550,-14340 }, { -10550,-19250 }, { - 9450,-14340 }, { - 9450,-19250 },/*24 右の下４つ */
    } ;
    FVECTOR *pos ;

    if ( work->flag & ORGA_F_UNRECOG_AIM || CHECKACT( work ) )
	pos = ORG_RecogPlayerPosNoHide( ORGA_GET_PLY_CONTROL ) ;
    else
	pos = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
    work->ply_locate  = ( (pos->vx - area[ 0][X]) * (area[ 0][Y] - area[12][Y]) >
			  (pos->vz - area[ 0][Y]) * (area[ 0][X] - area[12][X]) ) ? 0x0001 : 0x0 ;
    work->ply_locate |= ( (pos->vx - area[ 2][X]) * (area[ 2][Y] - area[14][Y]) >
			  (pos->vz - area[ 2][Y]) * (area[ 2][X] - area[14][X]) ) ? 0x0002 : 0x0 ;

    work->ply_locate |= ( (pos->vx - area[ 4][X]) * (area[ 4][Y] - area[12][Y]) >
			  (pos->vz - area[ 4][Y]) * (area[ 4][X] - area[12][X]) ) ? 0x0004 : 0x0 ;
    work->ply_locate |= ( (pos->vx - area[ 4][X]) * (area[ 4][Y] - area[14][Y]) >
			  (pos->vz - area[ 4][Y]) * (area[ 4][X] - area[14][X]) ) ? 0x0008 : 0x0 ;

    work->ply_locate |= ( (pos->vx - area[ 4][X]) * (area[ 4][Y] - area[19][Y]) >
			  (pos->vz - area[ 4][Y]) * (area[ 4][X] - area[19][X]) ) ? 0x0010 : 0x0 ;
    work->ply_locate |= ( (pos->vx - area[ 5][X]) * (area[ 5][Y] - area[17][Y]) >
			  (pos->vz - area[ 5][Y]) * (area[ 5][X] - area[17][X]) ) ? 0x0020 : 0x0 ;
    work->ply_locate |= ( (pos->vx - area[ 1][X]) * (area[ 1][Y] - area[19][Y]) >
			  (pos->vz - area[ 1][Y]) * (area[ 1][X] - area[19][X]) ) ? 0x0040 : 0x0 ;

    work->ply_locate |= ( (pos->vx - area[ 8][X]) * (area[ 8][Y] - area[ 1][Y]) >
			  (pos->vz - area[ 8][Y]) * (area[ 8][X] - area[ 1][X]) ) ? 0x0080 : 0x0 ;
    work->ply_locate |= ( (pos->vx - area[11][X]) * (area[11][Y] - area[19][Y]) >
			  (pos->vz - area[11][Y]) * (area[11][X] - area[19][X]) ) ? 0x0100 : 0x0 ;
}


static inline int ORG_RecogHeShowsUp( Work *work )
{
    extern HIDE ORG_HideBranch[] ;
    extern int ORG_OBJ_CanISeeYou( DG_OBJS *o, FVECTOR *snake, FVECTOR *orga ) ;
    float    d, w ;
    HIDE    *h = ORG_HideBranch ;
    int      i ;
    FVECTOR *snake, *orga, l ;

    /* 隠れてないで,ある程度の場所にいたらみていることになる */
    snake = ORG_RecogPlayerPosNoHide( HUMAN21_KUBI ) ;
    orga = BODYPOS( &ORG_Work->body, HUMAN21_ATAMA ) ;
    for ( i=ORGA_N_HIDE_POOL ; --i>=0 ; h++ )
	if ( ORG_OBJ_CanISeeYou( h->objs, snake, orga ) )
	    break ;/*視線が遮られている*/

    if ( !(work->ply_hide & ORGA_F_PLYR_HIDING) && i<0 )
    {
	/* スネークにとってオルガの頭が見えていることになる */
	_sceVu0SubVector( &l, snake, orga ) ;
	d = sceVu0Sqrt( _sceVu0InnerProduct( &l, &l ) ) ;
	w = 0.00001f*d + 0.77f ;/* 距離に応じて認識角度を変えている 短い→視野広い */
	_sceVu0ScaleVector( &l, &l, 1/d ) ;
	d = _sceVu0InnerProduct( &l, (FVECTOR*)&BODYWORLD( &work->body, HUMAN21_ATAMA).m[Z] ) ;
	if ( d > w )
	{
	    if ( d>0.9f )
		return 1 ;/*目の前だ*/
	    else
		return 2 ;/*目の前じゃない*/
	}
    }

    return 0 ;
}

/*

  オルガがスネークを見えているかどうか（正確さは要らない軽い奴）
  狙えるかどうかは,ちゃんと壁に遮られているのかを検知している。

*/
static inline void ORG_RecogICanSeeHim( Work *work )
{
    if ( !(work->ply_hide & ORGA_F_PLYR_HIDING) )
    {
	/*少なくとも2秒以上見られると隠れていないことになる*/
	if ( (work->ply_hide & ORGA_F_PLYR_HIDEXP) > ORGA_F_PLYR_HIDTIC*2 ||
	     !(work->ply_hide & ORGA_F_PLYR_HIDLNG) )
	{
	    work->trgt_hid = *ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ; 
	    work->ply_hide &= ~ORGA_F_PLYR_HIDLNG ;
	    work->ply_hide &=  ORGA_F_PLYR_HIDCLR ;
	    return ;
	}
	/*0.25秒ごとにカウントアップする*/
	if ( !(GV_Time & 0x0f) )
	    if ( (work->ply_hide & ORGA_F_PLYR_HIDEXP) != ORGA_F_PLYR_HIDEXP )
		    work->ply_hide += ORGA_F_PLYR_HIDTIC ;
    }

    /*少なくとも１秒以上見えない状態を作らないと見えないことにはならない*/
    if ( !(GV_Time & 0x3f) )
	if ( (++work->ply_hide & ORGA_F_PLYR_HIDTIM) > 1 )
	    work->ply_hide |= ORGA_F_PLYR_HIDLNG ;

    if ( work->ply_hide & ORGA_F_PLYR_HIDLNG )
	work->flag |=  ORGA_F_UNRECOG_POS ;
}



/*

  ノイズ関知（マガジンを投げた時の処理）

*/
static inline void ORG_RecogHeardNoise( Work *work )
{
    FVECTOR l ;

    if ( GM_NoisePower > NOISE_ZERO )
    {
	if ( GM_NoiseStatus == NOISE_STATUS_MAG &&
	     work->flag & (ORGA_F_UNRECOG_AIM| ORGA_F_UNRECOG_POS) &&
	     !CHECKACT( work )  &&  work->trgt_nse.vw < 0.0f )
//	    (GM_NoisePosition.vx <= -10000.0f && GM_NoisePosition.vz <= -17300.0f)
	{
	    /* 音との距離を見る */
	    _sceVu0SubVector( &l, &GM_NoisePosition, &work->control.mov ) ;
	    if ( _sceVu0InnerProduct( &l, &l ) < ORGA_HEARD_RAD )
	    {
		//_sceVu0CopyVector( &work->trgt_nse, &GM_NoisePosition ) ;
		_sceVu0CopyVector( &work->trgt_aim_pos, &work->trgt_hid ) ;
		work->trgt_aim  = &work->trgt_aim_pos ;
		work->trgt_eye  = &work->trgt_hid ;
		work->head_mark = HMK2_TYPE_WHT_AT ; /* 白！マーク */
		work->trgt_nse.vw = 0.0f ;
		//work->voice     = ORGA_VO_FOUND ;
		work->voice_tim = 0 ;
	    }
	}
    }    
}



/*

  オルガがプレイヤーの位置を追っているかどうか

*/
static inline void ORG_RecogUnrecognizing( Work *work )
{
    /*この条件は orga_thk_attk.h の条件と同じでなければならない */
    if ( work->head_time>0 )
	if ( !(GV_Time & 0x3f) )
	    work->head_time-- ;

    if ( (work->act_flg & ORGA_F_GAP_ATTK)  &&
	 !(work->flag & ORGA_F_DMG_FARHIDE) &&
	 work->rage < 4 )
	work->flag |= ORGA_F_UNRECOG_AIM, work->exposed = 0 ;

    else if ( work->flag & (ORGA_F_UNRECOG_AIM| ORGA_F_UNRECOG_POS) )
    {
	/* 迷い解除の条件 */
	int astonished = ORG_RecogHeShowsUp( work ) ;/*目の前にいる場合*/

	/*プレイヤーがダメージを追った場合*/
	if ( GM_CheckPlayerStatus( PLAYER_DAMAGED ) )
	    astonished |= 8 ;
	/*プレイヤーが近くを撃った場合*/
	if ( ORGA_PLY_SHOOT && work->flag & ORGA_F_AIMED_NEAR)
	    astonished |= 4 ;
	/*プレイヤーがアホな場合*/
	if ( work->act_flg & (ORGA_F_ERUDE_ATTK| ORGA_F_GOUND_ATTK) )
	    astonished |= 1 ;
	if ( astonished )
	{
	    /* 驚いたので頭の上にマークを出す (驚き度によって変わる) */
	    if ( (astonished      && work->head_time==0) || 
		 (astonished&0x08 && work->head_time<10) || 
		 (astonished&0x04 && work->head_time<25) || 
		 (astonished&0x02 && work->head_time<30) )
	    {
		work->head_mark = HMK2_TYPE_RED_AT ; /* ！マーク */
		work->head_time = 30 ;/*この秒数だけ,ビックリした時以外のビックリマークは出ない*/
		if ( astonished&0x07 )
		    work->voice_tim = 5, work->voice = ORGA_VO_FOUND ;
		if ( astonished&0x08 )
		    work->voice_tim = 5, work->voice = ORGA_VO_FINDTEASE1 ;
	    }

	    /* プレーヤーを見つけたので各種フラグを変更する */
	    work->flag  &= ~(ORGA_F_UNRECOG_AIM| ORGA_F_UNRECOG_POS) ;
	    work->ply_hide &= ~ORGA_F_PLYR_HIDLNG ;
	    work->ply_hide &=  ORGA_F_PLYR_HIDCLR ;
	    if ( !(work->act_flg & ORGA_F_HOLO_HIDE_ATTK) )
		work->stll_tim = 0 ;

	    /*顔をプレイヤーに向け(trgt_eye), 隠れていた位置を知った(trgt_hid) */
	    work->trgt_eye = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
	    _sceVu0CopyVector( &work->trgt_hid, &GM_PlayerControl->mov ) ;
	    (*(int*)&work->trgt_hid.vw) = work->ply_hide & ORGA_F_PLYR_HIDPOS ;
	    if ( *work->act == ORG_ActFireP || *work->act == ORG_ActUnrecogFireP )
	    {
		work->trgt_nse.vw = -1.0f ;/* ノイズで来てたら切ってしまう */
		work->wait_sec = 1 ;
	    }
	}
    }
}



/*

  プレイヤーのプレイの仕方

*/
static inline void ORG_RecogPlayer( Work *work )
{
    if ( !(GV_Time & 0x3ff) )
	work->ply_time++ ;

    /* 動いたかどうか */
    if ( GM_CheckPlayerStatus( PLAYER_MOVE ) )
	work->ply_stop = 0 ;
    else if ( !(GV_Time & 0x0ff) )
	work->ply_stop++ ;
}



/*

  障害物のターゲットに対する回り込み処理

*/
static int CheckLineHitToBoxXZ( FVECTOR *from, FVECTOR *to, HIDE *box )
{
    FVECTOR s, v, d ;

    _sceVu0SubVector( &s, to, from ) ;
    _sceVu0Normalize( &d, &s ) ;
    _sceVu0SubVector( &v, &box->center, from ) ;
    _sceVu0ScaleVector( &v, &d, (d.vx*v.vx + d.vz*v.vz) ) ;
    if ( s.vx*v.vx < 0.0f  && s.vz*v.vz < 0.0f )/* from と to の間にない */
	return 0 ;
    if ( s.vx*s.vx + s.vz*s.vz < v.vx*v.vx + v.vz*v.vz )/* from と to の間にない */
	return 0 ;
    _sceVu0AddVector( &v, &v, from ) ;
    _sceVu0SubVector( &v, &v, &box->center ) ;
    if ( (int)(v.vx/(box->size.vx+ORGA_HIDE_CHK_SPHERE)) ||
	 (int)(v.vz/(box->size.vz+ORGA_HIDE_CHK_SPHERE)) )
	return 0 ;
    return 1 ;    /* 直線 と ターゲット が 当たった */
}

static inline void ORG_RecogAvoidWall( Work *work )
{
    int     i, j ;
    int     id=-1, msk=-1 ;
    HIDE   *h = work->hide_pool ;
    static  FVECTOR next[4] ;
    float   min, t ;
    HIDE   *trgt ;

#if 0 /* バギーなので とりあえず外す */
    /* スネークが動くと目標場所がどんどん変わるので再計算する */
    if ( work->hide_spot && *work->act != ORG_ActStillStart )
    {
	int flag ;

	flag = ORG_RenewHidePosZ( work->hide_spot,
				  ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ),
				  work->stll_mtn ) ;
	/* 目標の位置が なくなってしまったので場所を変更する */
	if ( ((work->flag & ORGA_F_DANGR_LEFT ) && !(flag & 1) ) ||
	     ((work->flag & ORGA_F_DANGR_RIGHT) && !(flag & 2) ) )
		ORG_SearchHidePlace( work, work->stll_mtn ) ;
    }
#endif
    work->trgt_nxt = (void *)(trgt = NULL) ;

    /* ホロが舞っていなかったら 現在の地点から目標の地点で障害物があるかどうか */
    for ( i=0, min=ORGA_MAX_FLOAT ; i<work->n_hide_pool ; h++, i++ )
	if ( CheckLineHitToBoxXZ( &work->control.mov, work->trgt_pos, h ) )
	    if ( (t = VectorSquare( work->trgt_pos, &h->center )) < min )
		trgt = h, min = t, id = i ;

    /* ホロが開いているか どうかかのチェック */
    if ( ORG_RecogCheckHoloValid( work ) )
    {
	/* ホロが開いていたら特別なハイドでチェックする */
	if ( id == -1 )
	    if ( CheckLineHitToBoxXZ( &work->control.mov, work->trgt_pos, &ORG_HoloHide ) )
		trgt = &work->hide_pool[id = 2] ;
	/* ホロのハイドを通る時は,自分の居る場所で制限を掛ける */
	if ( id == 2 )
	{
	    float x = work->control.mov.vx - trgt->center.vx ;
	    float z = work->control.mov.vz - trgt->center.vz ;
	    if ( work->control.mov.vz > trgt->center.vz )
	    {
		if ( work->control.mov.vx < trgt->center.vx )
		{
		    msk <<= 4, msk |= 3 ;
		    if ( -z/x < trgt->size.vz/trgt->size.vx )
			msk <<= 4, msk |= 1 ;
		}
		else
		{
		    msk <<= 4, msk |= 1 ;
		    if ( z/x < trgt->size.vz/trgt->size.vx )
			msk <<= 4, msk |= 3 ;
		}
	    }
	}
    }

    /* 障害物があれば,メリ込まない十分な四点を通って回避する。なければ目標に向かって進む */
    if ( trgt )
	for ( i=4, min=ORGA_MAX_FLOAT ; --i>=0 ; )
	{
	    /* ホロが舞ったら 舞っている場所は通らない */
	    if ( id == 2 && (i==((msk>>0)&0xf) || i==((msk>>4)&0xf)) )
		continue ;
	    /* 低い隠れ場所の前は通らない */
	    if ( id == 1 && i&2 )
		continue ;
	    /* 前に通った場所は通らない */
	    for ( j=work->n_trgt_his ; --j>=0 ; )
		if ( work->trgt_his[j] == (u_int)trgt + i )
		    goto skip_history ;
	    /* 次の場所を選択 */
	    next[i].vx = trgt->center.vx + (i&2 ? trgt->size.vx+ORGA_HIDE_MOV_SPHERE :
					         -trgt->size.vx-ORGA_HIDE_MOV_SPHERE ) ;
	    next[i].vz = trgt->center.vz + (i&1 ? trgt->size.vz+ORGA_HIDE_MOV_SPHERE :
					         -trgt->size.vz-ORGA_HIDE_MOV_SPHERE ) ;
	    /* 現在の場所から一番近い所を次のポイントとする */
	    if ( (t = VectorSquare( &next[i], &work->control.mov )) < 400.0f*400.0f )
		work->trgt_his[work->n_trgt_his++] = (u_int)trgt + i ;
	    else if ( t < min )
		work->trgt_nxt = &next[i], min = t ;
	skip_history:
		;
	}
    work->n_hide_pool = ORGA_N_HIDE_DFLT ;
}


/*

  自分の命の状態を把握

 */
static inline void ORG_RecogVitality( Work *work )
{
    /* 麻酔を打たれた後,しばらくの間ダメージを受け続ける */
    if ( work->bit_dmg > 0 )
	if ( !(GV_Time & 0x7f) )
	{
	    if ( work->vitality_m9 <= ORGA_BIT_DAMAGE_MIN ) /* これ以上ダメージが減らない */
		work->bit_dmg = 0 ;
	    else
	    {
		if ( !(work->bit_dmg -= ORGA_BIT_DAMAGE) )/* 減りが終る時の気合い声 */
		    /*work->voice_tim = 1, work->voice = ORGA_VO_SHOTTEASE1*/ ;
		ORG_DamageVitality( work, ORGA_BIT_DAMAGE, 1/*M92ダメージ*/ ) ;
	    }
	}

    if ( work->vitality <= ORGA_MAX_VITALITY/8+5 || work->vitality_m9 <= ORGA_MAX_VITALITY/8+5 ) /* かなり弱っている */
    {
	work->act_speed = ORGA_NORMAL_SPEED*3 ;
	work->flag &= ~ORGA_F_LIFE_MASK ;
	work->flag |= ORGA_F_GOT_WORSE| ORGA_F_GOT_SICK ;
    }
    else if ( work->vitality <= ORGA_MAX_VITALITY/4+5 || work->vitality_m9 <= ORGA_MAX_VITALITY/4+5 ) /* 弱っている */
    {
	work->act_speed = ORGA_NORMAL_SPEED*3 ;
	work->flag &= ~ORGA_F_LIFE_MASK ;
	work->flag |= ORGA_F_GOT_WORSE ;
    }
    else if ( work->vitality <= ORGA_MAX_VITALITY/2+5 || work->vitality_m9 <= ORGA_MAX_VITALITY/2+5 ) /* 半分まで弱っている */
    {
	work->act_speed = ORGA_NORMAL_SPEED*2 ;
	work->flag &= ~ORGA_F_LIFE_MASK ;
	work->flag |= ORGA_F_GOT_SICK  ;
    }
    else
	work->act_speed = ORGA_NORMAL_SPEED ;
}


/*

 ダメージを受けていない時間

 */
static inline void ORG_RecogNonDamageTime( Work *work )
{
    work->flag &= ~ORGA_F_LEVEL_MSK ;
    if ( !(GV_Time & 0xff) )
	if ( work->non_dmg < ORGA_MAX_NONDMG_TIME )
	    work->non_dmg++ ;
    if ( work->non_dmg < ORGA_MAX_NONDMG_TIME/4 )
	work->flag |= ORGA_F_LEVEL_MANIA ;
    else if ( work->non_dmg < ORGA_MAX_NONDMG_TIME/2  || GM_GameLevel>=GM_LEVEL_HARD )
	work->flag |= ORGA_F_LEVEL_HARD ;
    else if ( work->non_dmg < ORGA_MAX_NONDMG_TIME*3/4 || GM_GameLevel>=GM_LEVEL_NORMAL )
	work->flag |= ORGA_F_LEVEL_NORM ;
    else
	work->flag |= ORGA_F_LEVEL_EASY ;
    
}

static inline void ORG_RecogStreaming( Work *work )
{
    
    if ( (work->voice_vox>>12) < ORGA_N_STREAM && !work->str_hdl )
    {
	if ( !(GV_Time & 0x3f) )
	    work->voice_vox++ ;

	/* 戦闘中での人生語り */
	if ( (work->voice_vox & 0xfff) >= ORGA_VOX_INTREVAL*0+1 &&
	     (work->body.m_ctrl->mt3_ctrl[0].motion_num < damg_leg_l ||
	      work->body.m_ctrl->mt3_ctrl[0].motion_num > damg_over  ) &&
	     (work->voice_vox>>12) != 0 && (work->voice_vox>>12) != ORGA_N_STREAM-1 )
	{
	    ORG_RecogStartStream( work ) ;
	    work->voice_vox += 0x1000 ;
	    work->voice_vox &= 0xf000 ;
	}
    }
}



/*

  特別行動に移るかどうかの判定

*/
static inline void ORG_RecogAbortCurrentAction( Work *work )
{
    work->act_flg &= 0x00010000 ;
    /* カットイン用の特別行動のみ残す */
    if ( work->flag & ORGA_F_DEMO_MOVIE )
	return ;
    
    /* 人生語り カットインなので 何よりも優先が高い */
    if ( (work->voice_vox & 0xfff) >= ORGA_VOX_INTREVAL &&
	 ((work->voice_vox>>12) == 0 || (work->voice_vox>>12) == ORGA_N_STREAM-1 ) )
    {
	/*ストリーム開始は action/org_teaz.hで*/
	work->act_flg |= ORGA_F_BIBLIO_ATTK ;
	if ( work->trgt_pos != &ORG_Ply_AttkBib[work->voice_vox>>12] )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }

    /* アホなスネークがエルードしているから撃ちにいく */
    else if ( GM_CheckPlayerStatus( PLAYER_BEYOND ) )
    {
	work->act_flg |= ORGA_F_ERUDE_ATTK ;
	if ( work->trgt_pos != &ORG_ShootErudePos )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }

#if 0
    /* 禁止区域で這っている */
    else if ( GM_PlayerControl->mov.vx <= -10000.0f && GM_PlayerControl->mov.vz <= -12700.0f )
    {
	if ( GM_CheckPlayerStatus( PLAYER_GROUND ) )
	{
	    work->act_flg |= ORGA_F_GOUND_ATTK ;
	    if ( work->trgt_pos != &ORG_Ply_AttkGrnd )
		work->act = ORG_ActionReset ; /* 実行を中断 */
	}
	else
	{
	    work->act_flg |= ORGA_F_STAND_ATTK ;
	    if ( work->trgt_pos != &ORG_Ply_AttkStnd )
		work->act = ORG_ActionReset ; /* 実行を中断 */
	}
    }
#endif

    /* ホロのヒラヒラに隠れて撃つ */
    else if ( !(work->flag & ORGA_F_DMG_HOROATTK) && ORG_RecogCheckHoloValid( work ) )
    {
	/* 移動するため実行を中断 */
	if (  work->trgt_pos != &work->hide_pool[2].right &&
	      work->trgt_pos != &ORG_Ply_AttkHoloL )
	    work->act = ORG_ActionReset ;

#if 1
	/* はみ出てたら移動 */
       else if ( work->ply_locate>=0x170 && !( work->flag & ORGA_F_ENB_ROUTE) &&
            work->trgt_pos == &work->hide_pool[2].right )
       {
           float z = ORG_GetHidePosZ( &work->hide_pool[2],
                                      ORG_RecogPlayerPosNoHide( ORGA_GET_PLY_CONTROL ),
                                     -ORGA_HIDE_H_SPHERE,
                                      work->stll_mtn ) ;
           if ( (int)((work->control.mov.vz - z)/400.0f) )
               work->act = ORG_ActionReset ; /* 強制的に移動 */
       }
       else if ( work->ply_locate>=0x170 ?
                 work->trgt_pos != &work->hide_pool[2].right :
		 work->trgt_pos != &ORG_Ply_AttkHoloL )
	   work->act = ORG_ActionReset ; /* 強制的に移動 */
#endif
	work->act_flg |= ORGA_F_HOLO_HIDE_ATTK ;
    }

    /* ホロフッ飛ばす もういらない */
    else if ( (work->voice_vox>>12)>=ORGA_N_STREAM &&
	      work->flag & (ORGA_F_GOT_SICK|ORGA_F_GOT_WORSE) &&
	      ORG_RecogCheckHoloBlowValid( work ) &&
	      ORG_RecogCheckSpotLight( work ) )
    {
	work->act_flg |= ORGA_F_HOLO_BLOW_ATTK ;
	if ( work->trgt_pos != &ORG_Ply_AttkHoloBlow )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }
    

    /* ホロ撃ちから戻ってくる時は,ホロを飛ばす */
    else if ( work->flag & ORGA_F_DMG_HOROATTK &&
	      ORG_RecogCheckHoloValid( work ) )
    {
	work->act_flg |= ORGA_F_HOLO_RUN_ATTK ;
	if ( work->trgt_pos != &ORG_ShootHoloRun1 &&
	     work->trgt_pos != &ORG_ShootHoloRun2 )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }


    /* 投光器をスネークに向け視界の邪魔をする */
    else if ( (work->voice_vox>>12)>=ORGA_N_STREAM &&
//#ifdef JAPANESE_BP_IGNORE()
	      ( !BP_Area_JP() || GM_GameLevel>=GM_LEVEL_NORMAL ) && 
//#endif
	      work->flag & (ORGA_F_GOT_SICK|ORGA_F_GOT_WORSE) &&
	      work->flag & ORGA_F_DMG_HOROATTK &&
	      !ORG_RecogCheckHoloValid( work )&&
	      ORG_RecogCheckSpotLight( work ) )
    {
	work->act_flg |= ORGA_F_SPOTLGT_ATTK ;

	if ( work->trgt_pos != &ORG_Ply_AttkLightL && work->trgt_pos != &ORG_Ply_AttkLightR )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }

    /* 一つの行動に時間が立ち過ぎ,現在のアクトを中断 */
    else if ( work->act_time > 10*10 )
	work->act = ORG_ActionReset ;

    /* すき間からスネークがしゃがんでいる */
    else if ( GM_CheckPlayerStatus( PLAYER_GROUND|PLAYER_SQUAT ) &&
	      !(((GM_PlayerControl->turn.vy & 4095) - 3072)/512) &&
	      GM_PlayerControl->mov.vx >=  -9000.0f &&
	      GM_PlayerControl->mov.vx <=  -7500.0f &&
	      GM_PlayerControl->mov.vz >= -17300.0f &&
	      GM_PlayerControl->mov.vz <= -16400.0f )
    {
	work->act_flg |= ORGA_F_GAP_ATTK ;
	if ( work->trgt_pos != &ORG_Ply_AttkPosL &&
	     work->trgt_pos != &ORG_Ply_AttkPosR )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }

    /* 音がしたので音の方向に顔を向ける */
    else if ( work->trgt_nse.vw >= 0.0f )
    {
	work->act_flg |= ORGA_F_MAGAZIN_ATTK ;
	if ( work->trgt_nse.vw != 1.0f )
	{
	    work->trgt_nse.vw = 1.0f ;
	    work->act = ORG_ActionReset ; /* 現在の実行を中断 */
	}
    }

    /* ホロを撃ちに行く (セリフを終ってから) */
    else if ( (work->voice_vox>>12)>=ORGA_N_STREAM &&
	      work->flag & (ORGA_F_GOT_SICK|ORGA_F_GOT_WORSE) &&
	      (work->misc_holo_stat ? *work->misc_holo_stat == ORGA_HOL_COVERED : 0 ) )
    {
	work->act_flg |= ORGA_F_HOLO_ATTK ;
	if ( work->trgt_pos != &ORG_ShootHoloPos && work->trgt_pos != &ORG_ShootHoloPos2 )
	    work->act = ORG_ActionReset ; /* 実行を中断 */
    }

    /* 特別な場所に来ているから取り敢えず移動する（全ての特別アクトが適応されない時） */
    else if ( work->trgt_pos == &ORG_ShootHoloPos   ||
	      work->trgt_pos == &ORG_ShootHoloPos2  ||
	      work->trgt_pos == &ORG_Ply_AttkPosL   ||
	      work->trgt_pos == &ORG_Ply_AttkPosR   ||
	      work->trgt_pos == &ORG_Ply_AttkGrnd   ||
	      work->trgt_pos == &ORG_Ply_AttkStnd   ||
	      work->trgt_pos == &ORG_Ply_AttkHoloL  ||
	      work->trgt_pos == &ORG_ShootErudePos  ||
	      work->trgt_pos == &ORG_Ply_AttkBib[0] ||
	      work->trgt_pos == &ORG_Ply_AttkBib[1] ||
	      work->trgt_pos == &ORG_Ply_AttkBib[2] ||
	      work->trgt_pos == &ORG_Ply_AttkBib[3] )
	work->act = ORG_ActionReset ; /* 現在の特別アクトをを中断 */

    /* あまりに露出時間長過ぎ */
#if 1
    else if ( !(work->flag & (ORGA_F_UNRECOG_AIM|ORGA_F_ENB_ROUTE)) &&
	      work->exposed >= 120 &&
	      !(work->ply_hide & ORGA_F_PLYR_HIDING) )
#else
    else if ( !(work->flag & ORGA_F_UNRECOG_AIM) && work->exposed > 0 )
#endif
	work->act = ORG_ActionReset ; /* 実行を中断 */


    /* 投光器フェーズが中断されているのでアクトを寝かす */
    if ( !(work->act_flg & ORGA_F_SPOTLGT_ATTK) )
	work->act_flg &= ~ORGA_F_CHNGLGT_ATTK ;
}


/*

  行動している時間

  */
static inline void ORG_RecogActingTime( Work *work )
{
    if ( ++work->act_tic > 30/TIME_BASE )
    {
	work->act_time++ ;
	work->act_tic = 0 ;
    }
}


/*
  行動を取るための情報収集。

  ほぼ全ての情報がここで分かりやすい情報に加工される。
  フラグとしての情報でいい場合ものは,work->flagのビットして情報の加工が行なわれている。

  Recognizing whole my state to prepare for thinking!!

*/
void ORG_Recognition( Work *work )
{
    /* 生きている時しか関知しなくていいもの */
    if ( work->vitality > 0 && work->vitality_m9 > 0 )
    {
	if ( !GM_IsGameOver() )
	{
	    /* 行動時間                               */
	    ORG_RecogActingTime( work ) ;

	    /* 自分の命の状態                         */
	    ORG_RecogVitality( work ) ;
	    /* ダメージを受けていない時間             */
	    ORG_RecogNonDamageTime( work ) ;

	    /* 近くにマガジンが落ちたかどうかを調べる */
	    ORG_RecogHeardNoise( work ) ;
	    /* ストリームをさせるかどうか             */
	    ORG_RecogStreaming( work ) ;
	    /* 自分がスネークに狙われているかどうか   */
	    ORG_RecogAimedBy( work ) ;
	    /* 自分がスネークを狙えるかどうか (プレーヤーが撃てば当たる?)*/
	    if ( work->flag & ORGA_F_ENB_AIM )
		ORG_RecogCanAim( work ) ;

	    /* オルガからスネークが見えているかどうか */
	    ORG_RecogICanSeeHim( work ) ;

	    /* プレイヤーの情報                       */
	    ORG_RecogPlayer( work ) ;
	    /* プレイヤーの大まかな位置               */
	    ORG_RecogHideStatus( work ) ;    
	    /* プレイヤーの位置をオルガが捉えているか */
	    ORG_RecogUnrecognizing( work ) ;

	    /* 自分が安全地帯にいるかどうか           */
	    if ( work->flag & (ORGA_F_ENB_ROUTE | ORGA_F_UNRECOG_AIM) )
		work->exposed = 0 ;
	    else if ( work->flag & ORGA_F_ENB_SAFE &&
		      work->hide_spot && work->trgt_pos )
		ORG_RecogSafeZone( work ) ;

	    /* 行動の中断をするべきかの判断           */
	    if ( !(work->act_flg & ORGA_F_INVALID_ATTK) &&
		 !(work->flag    & ORGA_F_NVR_RESET   ) )
		ORG_RecogAbortCurrentAction( work ) ;
	    /* 移動の時の障害物を避ける               */
	    if ( work->flag & ORGA_F_ENB_ROUTE && work->trgt_pos )
		ORG_RecogAvoidWall( work ) ;
	    else
		work->trgt_nxt = NULL, work->n_trgt_his = 0 ;
	}
	else
	{
	    if ( work->str_hdl )
		ORG_RecogStopStream( work ) ;

	    /* オルガが勝ってゲームオーバーになった */
	    if ( work->act_flg != ORGA_F_GAMEOVER_ATTK )
	    {
		work->act_flg = ORGA_F_GAMEOVER_ATTK ;
		work->act = ORG_ActionAbort ;
	    }
	    _sceVu0CopyVector( &ORG_HereIam, &work->control.mov ) ;
	    work->trgt_pos = &ORG_HereIam ;
	}

    }

    /* デモなのでグレネードなどの武器は爆発しない */
    if ( work->flag & ORGA_F_DEMO_MOVIE )
	work->weap_sgr.vw = -4.0f ;
}

