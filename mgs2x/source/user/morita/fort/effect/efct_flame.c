//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_flame.c
  フォーチュン戦 炎のエフェクト

  2001/02/08 T.Morita
  $Id: efct_flame.c,v 1.1.1.3 2002/11/19 11:46:10 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../../include/util.h"
#include "../../include/libdg_x.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"

#define FLOOR_FLAME 0 


#define FLAME_SIZE   600.0f
#define FLAME_SPEED  30.0f
#define FLAME_N_FRAME 12
#define FLAME_N_FLOOR 12
#define FLAME_N_PRIM  (FLAME_N_FRAME)
#define BRK_GRAVITY   6.0f

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct vol_t
{
    FMATRIX   world1 ;
    FMATRIX   world2 ;
    FMATRIX   worlds ; /* 煙用 */

    float     vel [FLAME_N_FRAME] ;
    short     tics[FLAME_N_FRAME] ;
    short     rot [FLAME_N_FRAME] ;
    short     alph[FLAME_N_FRAME] ;
    short     spray[FLAME_N_FRAME] ;

    int       alpha   ;

    DG_PRIM2 *prim1 ;
    DG_PRIM2 *prim2 ;
    DG_PRIM2 *fade1 ;
    DG_PRIM2 *fade2 ;
    DG_PRIM2 *smoke ;
} VOLUME ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    VOLUME    flame ;

    DG_PRIM2 *fly ;

#if FLOOR_FLAME
    DG_PRIM2 *floor ;
#endif

    int       sprayed ;
    int       where ;
    int       life ;
} Work ;



static FVECTOR Offset = {
    FLAME_SIZE/2.0f,
    FLAME_SIZE/2.0f,
    FLAME_SIZE/2.0f,
    0.0f
}  ;




#if FLOOR_FLAME
static int ActFloor( Work *work, VOLUME *vol )
{
    FVECTOR        *prv_p, *nxt_p ;
    DG_PRIM2_UVRGB *prv_u, *nxt_u ;
    DG_PRIM2 *p = work->floor ;
    int       i ;
    int       flag = 0 ;
    int       alpha = 1 ;

    prv_p = p->pos  [p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos  [p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;

    for ( i=FLAME_N_FLOOR ; --i>=0 ; nxt_p++, prv_p++, nxt_u++, prv_u++ )
    {
	if ( (nxt_u->a = prv_u->a) )
	{
	    nxt_p->vx = prv_p->vx + 5.0f * frnd() ;
	    nxt_p->vy = prv_p->vy + vol->vel[i]*0.05f ;
	    nxt_p->vz = prv_p->vz + 5.0f * frnd() ;
	    if ( nxt_p->vy > 20.0f )
		nxt_u->a -= nxt_u->a>alpha ? alpha : nxt_u->a ;
	    else if ( nxt_u->a < 200 )
		nxt_u->a += alpha ;
	}
	else if ( work->life )
	{
	    nxt_u->a = 1 ;
	    nxt_p->vx = FLAME_SIZE * 0.5f * frnd() ;
	    nxt_p->vy = 0.0f ;
	    nxt_p->vz = FLAME_SIZE * 0.5f * frnd() ;
	}
	_sceVu0CopyVector( nxt_p+1, prv_p ) ;
	flag |= nxt_u->a ;
    }

    return flag ;
}
#endif


static int ActFly( Work *work, VOLUME *vol )
{
    FVECTOR        *prv_p, *nxt_p ;
    DG_PRIM2_UVRGB *prv_u, *nxt_u ;
    DG_PRIM2 *p = work->fly ;
    int       i ;
    int       flag = 0 ;

    prv_p = p->pos  [p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos  [p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;

    for ( i=FLAME_N_FRAME ; --i>=0 ; )
    {
	if ( (nxt_u->a = prv_u->a) )
	{
	    nxt_p->vx = prv_p->vx + 5.0f * frnd() ;
	    nxt_p->vy = prv_p->vy + vol->vel[i] ;
	    nxt_p->vz = prv_p->vz + 5.0f * frnd() ;
	    if ( nxt_p->vy > 2000.0f )
		nxt_u->a-- ;
	    else if ( nxt_u->a < 200 )
		nxt_u->a++ ;
	}
	else if ( work->life )
	{
	    nxt_u->a = 1 ;
	    nxt_p->vx = FLAME_SIZE * 0.5f * frnd() ;
	    nxt_p->vy = 0.0f ;
	    nxt_p->vz = FLAME_SIZE * 0.5f * frnd() ;
	}
	_sceVu0CopyVector( nxt_p+1, prv_p ) ;
	flag |= nxt_u->a ;
	nxt_p += 2 ; prv_p += 2 ;
	nxt_u += 2 ; prv_u += 2 ;
    }

    return flag ;
}

static int ColdSprayForce = 0 ;

#define	SPRAY_LENGTH	(3000.0f)
#define	INNER_LEVEL	(0.95f)
static inline void IsSprayed( Work *work )
{
    static FVECTOR NozzlePoint = { 13.0f, -323.0f, 61.0f, 1.0f };
    FVECTOR  v    ;
    FMATRIX *root ;
    float det ;

    if ( ColdSprayForce )
    {
	if ( ColdSprayForce   != GM_StagePlayTime &&
	     ColdSprayForce-1 != GM_StagePlayTime )
	    ColdSprayForce = 0 ;
	work->sprayed = -60*5/TIME_BASE ;
    }

    /* 冷却スプレー消火 */
    if ( GM_WeaponFire == WP_ColdSpray )
    {
	if ( GM_PlayerSubWeaponBody != NULL )
	{
	    /* 冷却スプレーのマトリクス取得 */
	    root = GM_PlayerSubWeaponBody->objs->root;
	    /* ノズルの絶対位置取得 */
	    DG_SetPos( root );
	    DG_PutVector( &NozzlePoint, &v, 1 );

	    /* 距離判定 */
	    _sceVu0SubVector( &v, &v, (FVECTOR *)work->flame.world1.m[W] ) ;
	    if ( _sceVu0InnerProduct( &v, &v ) < SPRAY_LENGTH * SPRAY_LENGTH )
	    {	
		/* 角度判定 */
		_sceVu0Normalize( &v, &v );
		det = _sceVu0InnerProduct( &v, (FVECTOR *)root->m[Y] ) ;
		if ( det > INNER_LEVEL )
		    if ( ++work->sprayed > 2*60*5/TIME_BASE )
		    {
			work->sprayed = -2*60*5/TIME_BASE ;
			ColdSprayForce = GM_StagePlayTime+1 ;
		    }
	    }
	}
    }
    else
	work->sprayed = 0 ;
}

static int ActFlame( Work *work, VOLUME *vol, int n_volume )
{
    FVECTOR          *prv_p1, *nxt_p1 ;
    DG_PRIM2_UVRGBWH *prv_u1, *nxt_u1, *nxt_u2  ;
    DG_PRIM2_UVRGBWH *prv_fu1,*nxt_fu1,*nxt_fu2 ;
    DG_PRIM2_UVRGBWH *nxt_us ;
    int   i ;
    int   flag = 0 ;

    prv_p1  = vol->prim1->pos  [vol->prim1->buffer_clock] ;
    prv_u1  = vol->prim1->uvrgb[vol->prim2->buffer_clock] ;
    prv_fu1 = vol->fade1->uvrgb[vol->fade1->buffer_clock] ;
    DG_SwitchBuffPrim2( vol->prim1 ) ;
    DG_SwitchBuffPrim2( vol->prim2 ) ;
    DG_SwitchBuffPrim2( vol->fade2 ) ;
    DG_SwitchBuffPrim2( vol->fade1 ) ;
    DG_SwitchBuffPrim2( vol->smoke ) ;
    nxt_p1  = vol->prim1->pos  [vol->prim1->buffer_clock] ;
    nxt_u1  = vol->prim1->uvrgb[vol->prim1->buffer_clock] ;
    nxt_u2  = vol->prim2->uvrgb[vol->prim2->buffer_clock] ;
    nxt_fu1 = vol->fade1->uvrgb[vol->fade1->buffer_clock] ;
    nxt_fu2 = vol->fade2->uvrgb[vol->fade2->buffer_clock] ;
    nxt_us  = vol->smoke->uvrgb[vol->smoke->buffer_clock] ;

    for ( i=n_volume ; --i>=0 ; )
    {
	nxt_u1 [i].a = prv_u1 [i].a ;
	nxt_fu1[i].a = prv_fu1[i].a ;

	if ( vol->tics[i] == 0 )
	{
	    nxt_u1[i].a  = 1 ;
	    nxt_u1[i].w  = nxt_u1[i].h = nxt_u2[i].w = nxt_u2[i].h = 0 ;
	    nxt_p1[i].vx = FLAME_SIZE * 0.5f * frnd() ;
	    nxt_p1[i].vy = 0.0f ;
	    nxt_p1[i].vz = FLAME_SIZE * 0.5f * frnd() ;
	    nxt_fu1[i].a = 0 ;

	    if ( work->life )
		vol->tics[i]  = 1 ;
	    vol->alph[i]  = 0       ;
	    vol->spray[i] = work->sprayed ;
	}
	else if ( vol->tics[i]++ > 0 )/*if ( nxt_u1[i].a || nxt_fu1[i].a )*/
	{
	    float r, a ;

	    if ( vol->tics[i] > 60*2 )
	    {
		if ( --vol->alph[i] + vol->alpha <= 0 )
		    vol->tics[i] = 0 ;

		if ( vol->spray[i] <= 0 )
		    nxt_fu1[i].a = vol->alph[i]>0 ? 
			vol->alpha - vol->alph[i] : vol->alph[i] + vol->alpha ;
		else
		    nxt_fu1[i].a = 0 ;
	    }
	    else if ( vol->alph[i] < vol->alpha )
		vol->alph[i]++ ;

	    if ( vol->spray[i] <= 0 && vol->alph[i] > 0 )
		nxt_u1[i].a = vol->alph[i] ;
	    else
		nxt_u1[i].a = 0 ;

#define ANGtoRAD(_a,_b) (((_a)&(_b))*(float)M_PI/(((_b) >> 1)+1.0f))
	    a = ANGtoRAD( vol->tics[i], vol->rot[i] ) + ((int)work & 0xff) ;
	    r = vol->tics[i] + FLAME_SIZE ;

	    nxt_fu1[i].w = nxt_u1[i].w = (short)( r * vu0_Sin( a + i )) ;
	    nxt_fu1[i].h = nxt_u1[i].h = (short)( r * vu0_Cos( a + i )) ;
	    nxt_fu2[i].w = nxt_u2[i].w = (short)( r * vu0_Sin(-a + i )) ;
	    nxt_fu2[i].h = nxt_u2[i].h = (short)( r * vu0_Cos(-a + i )) ;
	    _sceVu0CopyVector( &nxt_p1[i], &prv_p1[i] ) ;
	    nxt_p1[i].vy += vol->vel[i] + 1024.0f*0.25f/(float)vol->rot[i] ;
	}


	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) && i&1 )
	    nxt_u1[i].a = nxt_fu2[i].a = 0 ;
	else
	{
	    nxt_u2[i].a  = nxt_u1[i].a >8 ? nxt_u1[i].a -8 : 0 ;
	    nxt_fu2[i].a = nxt_fu1[i].a>8 ? nxt_fu1[i].a-8 : 0 ;
	}
	if ( !nxt_u1[i].a )
	    nxt_u1[i].w = nxt_u1[i].h = nxt_u2[i].w = nxt_u2[i].h = 0 ;
	if ( !nxt_fu1[i].a )
	    nxt_fu1[i].w = nxt_fu1[i].h = nxt_fu2[i].w = nxt_fu2[i].h = 0 ;

	nxt_us[i].w = nxt_u1[i].w ;
	nxt_us[i].h = nxt_u1[i].h ;
	nxt_us[i].a = vol->alph[i]>0 ? vol->alph[i] : 0 ;

	flag |= nxt_u1[i].a | nxt_fu1[i].a | nxt_us[i].a ;
    }

    return flag ;
}

static void Act( Work *work )
{
    int flag ;

    IsSprayed( work ) ;

    flag  = ActFly  ( work, &work->flame ) ;
#if FLOOR_FLAME
    flag |= ActFloor( work, &work->flame ) ;
#endif
    flag |= ActFlame( work, &work->flame, FLAME_N_FRAME ) ;

    if ( work->life > 0 )
	work->life-- ;
    else if ( !work->life && !flag )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->flame.prim1 )
	GM_FreePrim2( work->flame.prim1 ) ;
    if ( work->flame.prim2 )
	GM_FreePrim2( work->flame.prim2 ) ;

    if ( work->flame.fade1 )
	GM_FreePrim2( work->flame.fade1 ) ;
    if ( work->flame.fade2 )
	GM_FreePrim2( work->flame.fade2 ) ;

    if ( work->flame.smoke )
	GM_FreePrim2( work->flame.smoke ) ;

    if ( work->fly )
	GM_FreePrim2( work->fly ) ;
#if FLOOR_FLAME
    if ( work->floor )
	GM_FreePrim2( work->floor ) ;
#endif
}


static int InitVolume( VOLUME *vol, FVECTOR *pos, int n_volume )
{
    int i ;
    int flame_id = 1444330 ; //StrCode( "w11c2_fire3a_alp" )
    int fade_id  = 2492906 ; //StrCode( "w11c2_fire3b_alp" )
    int smoke_id = 12186572 ; // GV_StrCode( "w11c2_smoke1_alp" ),
    u_long64 alpha ;
    u_int  rgba  ;


    /* 炎 */
    rgba  = 0x207f7f7f ;
    alpha = SCE_GS_SET_ALPHA(0,2,0,1,0) ;
    vol->alpha = (rgba >> 24) & 0xff ;
    rgba &= 0x00ffffff ;
    if ( !(vol->prim1 = BRK_UTL_MakeRSPRTWH( n_volume, flame_id,
					     alpha, 0.0f, rgba )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;
    vol->prim1->root = &vol->world1 ;
    if ( !(vol->prim2 = BRK_UTL_MakeRSPRTWH( n_volume, flame_id,
					     alpha, 0.0f, rgba )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;
    DG_SetPrim2Buffer( vol->prim2,
		       vol->prim1->pos[0]  , vol->prim1->pos[1],
		       vol->prim2->uvrgb[0], vol->prim2->uvrgb[1] ) ;
    vol->prim2->root = &vol->world2 ;


    /* フェードアウト用 */
    if ( !(vol->fade1 = BRK_UTL_MakeRSPRTWH( n_volume, fade_id ,
					     alpha, 0.0f, rgba )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;
    DG_SetPrim2Buffer( vol->fade1,
		       vol->prim1->pos[0]  , vol->prim1->pos[1],
		       vol->fade1->uvrgb[0], vol->fade1->uvrgb[1] ) ;
    vol->fade1->root = &vol->world1 ;
    if ( !(vol->fade2 = BRK_UTL_MakeRSPRTWH( n_volume, fade_id ,
					     alpha, 0.0f, rgba )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;
    DG_SetPrim2Buffer( vol->fade2,
		       vol->prim1->pos[0]  , vol->prim1->pos[1],
		       vol->fade2->uvrgb[0], vol->fade2->uvrgb[1] ) ;
    vol->fade2->root = &vol->world2 ;


    /* 煙用 */
    alpha = SCE_GS_SET_ALPHA(2,0,0,1,0) ;
    //vol->alpha_s = 0x1a ;
    rgba  = 0x007f7f7f ;
    if ( !(vol->smoke = BRK_UTL_MakeRSPRTWH( n_volume, smoke_id,
					     alpha, 0.0f, rgba )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;
    DG_SetPrim2Buffer( vol->smoke,
		       vol->prim1->pos[0]  , vol->prim1->pos[1],
		       vol->smoke->uvrgb[0], vol->smoke->uvrgb[1] ) ;
    vol->smoke->root = &vol->worlds ;


    _sceVu0CopyMatrix( &vol->world1, &DG_UnitMatrix ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR*)vol->world1.m[W], pos ) ;
    _sceVu0CopyMatrix( &vol->world2, &vol->world1 ) ;
    _sceVu0AddVector( (FVECTOR*)vol->world2.m[W], pos, &Offset ) ;
    _sceVu0CopyMatrix( &vol->worlds, &vol->world1 ) ;
    vol->worlds.m[W][Y] += 1000.0f ;
    vol->worlds.m[Y][Y] = 1.5f ;


    for ( i=n_volume ; --i>=0 ; )
    {
	vol->vel[i]  = rnd()*4.0f + 8.0f ;
	vol->rot[i]  = 0x1ff >> (irnd()&1) ;
	vol->tics[i] = -( i*18 + (int)(irnd() & 15) ) ;
    }

    return 0 ;
}


static void AleadyBurned( Work *work, VOLUME *vol, int n_volume )
{
    FVECTOR          *prv_p1 ,*nxt_p1 ;
    DG_PRIM2_UVRGBWH *prv_u1 ,*prv_u2 ,*nxt_u1 ,*nxt_u2  ;
    DG_PRIM2_UVRGBWH *prv_fu1,*prv_fu2,*nxt_fu1,*nxt_fu2 ;
    DG_PRIM2_UVRGBWH *prv_us ,*nxt_us ;
    int   i ;
    int   max  = 0 ;
    float r, a ;

    prv_p1  = vol->prim1->pos  [0], nxt_p1  = vol->prim1->pos  [1] ;
    prv_u1  = vol->prim1->uvrgb[0], nxt_u1  = vol->prim1->uvrgb[1] ;
    prv_u2  = vol->prim2->uvrgb[0], nxt_u2  = vol->prim2->uvrgb[1] ;
    prv_fu1 = vol->fade1->uvrgb[0], nxt_fu1 = vol->fade1->uvrgb[1] ;
    prv_fu2 = vol->fade2->uvrgb[0], nxt_fu2 = vol->fade2->uvrgb[1] ;
    prv_us  = vol->smoke->uvrgb[0], nxt_us  = vol->smoke->uvrgb[1] ;

    for ( i=n_volume ; --i>=0 ; )
	if ( vol->tics[i] < max )
	    max = vol->tics[i] ;

    for ( i=n_volume ; --i>=0 ; )
    {
	vol->tics[i] -= max ;
	if ( vol->tics[i] > 60*2 + vol->alpha*2 )
	    vol->tics[i] -= 60*2 + vol->alpha*2 ;

	/* αを初期化 */
	if ( vol->tics[i] < vol->alpha )
	    nxt_u1[i].a = vol->tics[i]+1, nxt_fu1[i].a = 0 ;
	else if ( vol->tics[i] < 60*2 )
	    nxt_u1[i].a = vol->alpha    , nxt_fu1[i].a = 0 ;
	else if ( vol->tics[i] < 60*2 + vol->alpha )
	    nxt_u1[i].a = vol->tics[i] - 60*2,
		nxt_fu1[i].a = vol->alpha - nxt_u1[i].a  ;
	else
	    nxt_u1[i].a = 0, nxt_fu1[i].a = vol->tics[i] - 60*2 - vol->alpha ;
	nxt_fu2[i].a = prv_fu1[i].a = prv_fu2[i].a = nxt_fu1[i].a ;
	nxt_u2[i].a  = prv_u1[i].a  = prv_u2[i].a  = nxt_u1[i].a  ;

	/* w,h を設定 */
	a = ANGtoRAD( vol->tics[i], vol->rot[i] ) + ((int)work & 0xff) ;
	r = vol->tics[i] + FLAME_SIZE ;
	nxt_u1[i].w  = prv_u1[i].w  = (short)(r * vu0_Sin( a + i )) ;
	nxt_u1[i].h  = prv_u1[i].h  = (short)(r * vu0_Cos( a + i )) ;
	nxt_u2[i].w  = prv_u2[i].w  = (short)(r * vu0_Sin(-a + i )) ;
	nxt_u2[i].h  = prv_u2[i].h  = (short)(r * vu0_Cos(-a + i )) ;
	nxt_fu1[i].w = prv_fu1[i].w = nxt_u1[i].w ;
	nxt_fu1[i].h = prv_fu1[i].h = nxt_u1[i].h ;
	nxt_fu2[i].w = prv_fu2[i].w = nxt_u2[i].w ;
	nxt_fu2[i].h = prv_fu2[i].h = nxt_u2[i].h ;

	/* 位置を初期化 */
	nxt_p1[i].vx = FLAME_SIZE * 0.5f * frnd() ;
	nxt_p1[i].vy = (vol->vel[i] + 1024.0f*0.25f/(float)vol->rot[i])*vol->tics[i] ;
	nxt_p1[i].vz = FLAME_SIZE * 0.5f * frnd() ;
	_sceVu0CopyVector( &prv_p1[i] , &nxt_p1[i] ) ;
    }
}


#if FLOOR_FLAME
static int InitFloorFire( Work *work )
{
    int  i ;

    if ( !(work->floor = BRK_UTL_MakeSPRTWH( FLAME_N_FLOOR,
					     1444330, //w11c2_fire3a_alp
					     SCE_GS_SET_ALPHA(0,2,0,1,0),
					     100.0f, 0x007f7f7f )) )
	PERROR( "No FloorPrim(no memory) : NewFortFlame\n" ) ;
    work->floor->root = &work->flame.world1 ;

    for ( i=FLAME_N_FLOOR ; --i>=0 ; )
    {
	FVECTOR *p0 = &work->floor->pos[0][i] ;
	FVECTOR *p1 = &work->floor->pos[1][i] ;

	p0->vx = FLAME_SIZE * frnd() ;
	p0->vy = 0.0f ;
	p0->vz = FLAME_SIZE * frnd() ;
	_sceVu0CopyVector( p1, p0 ) ;
    }
    return  0 ;
}
#endif

static int InitFlyFire( Work *work )
{
    int  i ;

    if ( !(work->fly = BRK_UTL_MakeLINE( FLAME_N_PRIM,
					 0,
					 SCE_GS_SET_ALPHA(0,2,0,1,0),
					 0x004f7f7f,
					 0x004f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;
    work->fly->root = &work->flame.world2 ;

    for ( i=FLAME_N_PRIM*2 ; --i>=0 ; )
    {
	FVECTOR *p0 = &work->fly->pos[0][i] ;
	FVECTOR *p1 = &work->fly->pos[1][i] ;

	p0->vx = FLAME_SIZE * 0.5f * frnd() ;
	p0->vy =           2000.0f *  rnd() ;
	p0->vz = FLAME_SIZE * 0.5f * frnd() ;
	_sceVu0CopyVector( p1  , p0 ) ;
    }
    return  0 ;
}

static int GetResources( Work *work, FVECTOR *pos, int life, int already )
{
    work->life = life ;
    if ( InitVolume( &work->flame, pos, FLAME_N_FRAME ) < 0 )
	return -1 ;

    if ( already )
	AleadyBurned( work, &work->flame, FLAME_N_FRAME ) ;

    if ( InitFlyFire( work ) < 0 )
	return -1 ;

#if FLOOR_FLAME
    if ( InitFloorFire( work ) < 0 )
	return -1 ;
#endif

    ColdSprayForce = 0 ;/* 初期化してしまう */

    return 0 ;
}

void *NewFortFlame( FVECTOR *pos, int life, int already )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, pos, life, already ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
