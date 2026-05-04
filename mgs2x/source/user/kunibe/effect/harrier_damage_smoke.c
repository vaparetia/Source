//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_damage_smoke.c
    ハリアーダメージ煙
    2001/05/10 Yuuta Kunibe	
    $Id: harrier_damage_smoke.c,v 1.1.1.3 2002/11/19 11:44:40 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define N_PRIMS	(160)

#define COLOR	(32)//(128)
#define ALPHA	(20)//(32)
#define SIZE	(100.0f)

#define	LIFE	(360)

inline int BP_AdjustTick2(int);
#define	RISING_SPEED	((float)BP_AdjustTick2(20))

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define WIND_MAX (20.f)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;


typedef	struct	{

    GV_ACT_EX	actor;

    int 	name;

    DG_PRIM2	*prim;

    FMATRIX	*mat;
    FVECTOR	shift;
    FVECTOR	demo_pos;
    FVECTOR	*pos;

    FVECTOR	vec[N_PRIMS];
    float	radius[N_PRIMS];
    float	cos[N_PRIMS];
    float	sin[N_PRIMS];

    float	size;
    float	initsize;
    float	rising_spd;
    FVECTOR	wind;

    float	init_alpha;

    char 	*flag;
    
    int  	id;
    int 	life;
    int		cnt;

    GV_MSG	*msg;

} Work;



static int RecieveKillMessage( Work *work )
{

    GV_MSG	*msg;
    int 	n_msg;


    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &work->msg ); 

    msg = work->msg;

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:
	    return 1;
	    break;
	}
	msg++;
    }

    return 0;

}



/* アクト関数 */
static void Act( Work *work )
{

    int			i,j;
    int 		n;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    float		intense;
    float		len;
    float		ftmp;
    float		angle;
    FVECTOR		vectmp;
    FVECTOR		prevpos;
    FVECTOR		unit_vec;
    FVECTOR		local_wind;
    int			alpha;
    

    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];


    /* 風計算 */
    intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;


    if ( work->mat != NULL ) {	/* デモ用位置更新 */

	DG_SetPos( work->mat );
	DG_PutVector( &work->shift, &work->demo_pos, 1 );

    }


    n = work->id - 1;
    if ( n < 0 ) n = N_PRIMS - 1;


    if ( work->flag != NULL ) {
	if ( *work->flag ) {
	    if ( work->init_alpha > 0 ) {
		work->init_alpha -= 0.2f;
	    }
	}
    }

    /* スプライト更新 */
    for ( i = 0 ; i < N_PRIMS ; i++ ) {	

	j = work->id - i;
	if ( j < 0 ) j += N_PRIMS;
	//alpha = ALPHA * ( N_PRIMS - j ) / N_PRIMS;
	alpha = work->init_alpha * ( N_PRIMS - j ) / N_PRIMS;
	
	ftmp = 1.0f + frnd() * 0.2f;
	_sceVu0ScaleVector( &vectmp, &work->wind, ftmp );
	_sceVu0AddVector( &pos[i], &pos_pre[i], &vectmp );

	pos[i].vy += work->rising_spd;

	
	uvrgbwh[i].a = alpha;
	if ( uvrgbwh[i].a > 0 ) {
	    ftmp = work->radius[i] * (float)( N_PRIMS - j ) / (float)N_PRIMS + work->radius[i] * 2.5f * (float)j / (float)N_PRIMS;
	    uvrgbwh[i].w = ftmp * work->cos[i];
	    uvrgbwh[i].h = ftmp * work->sin[i];
	}
	else {
	    uvrgbwh[i].w = 0;
	    uvrgbwh[i].h = 0;
	}
	/*if ( !( work->cnt % 3 ) ) {
	    if ( uvrgbwh_pre[i].a > 0 ) {
		uvrgbwh[i].a = uvrgbwh_pre[i].a - 1;
	    }
	    else {
		uvrgbwh[i].w = 0.f;
		uvrgbwh[i].h = 0.f;
		uvrgbwh[i].a = 0;
	    }
	}
	else {
	    uvrgbwh[i].a = uvrgbwh_pre[i].a;
	}*/
		
	if ( i == n ) DG_COPY_VEC( &prevpos, &pos[i] );
    }

    /* 前の煙との直線距離から発生させる煙の数を算出 */
    len = GV_VecLen3F2( &prevpos, work->pos );

    n = (int)( len / ( work->size / 3.f ) );
    n += 1;
    ftmp = 1.0f / (float)n;
    _sceVu0SubVector( &unit_vec, work->pos, &prevpos );
    _sceVu0ScaleVector( &unit_vec, &unit_vec, ftmp );

    
    work->initsize = work->initsize * ( 1.f + frnd()*0.1f );

    if (!(irnd()%120)) work->initsize = work->size * 1.25f;
    
    if ( work->initsize > work->size * 1.25f ) {
	work->initsize = work->size * 1.25f;
    }
    else if ( work->initsize < work->size * 0.8f ) {
	work->initsize = work->size * 0.8f;
    }

    
      
    /* 算出した個数分煙生成 */
    for ( i = 0 ; i < n ; i++ ) {
	DG_COPY_VEC( &pos[work->id], &prevpos );

	_sceVu0ScaleVector( &vectmp, &unit_vec, (float)(i+1) );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &vectmp );
	pos[work->id].vx += work->size / 4.f * frnd();
	pos[work->id].vy += work->size / 4.f * frnd();
	pos[work->id].vz += work->size / 4.f * frnd();

	work->radius[work->id] = work->initsize * ( 1.20f + frnd() * 0.40f );
	angle = TPI * frnd();
	work->cos[work->id] = vu0_Cos( angle );
	work->sin[work->id] = vu0_Sin( angle );
	uvrgbwh[work->id].w  = work->radius[work->id] * work->cos[work->id];
	uvrgbwh[work->id].h  = work->radius[work->id] * work->sin[work->id];
	
	uvrgbwh[work->id].a = (u_short)work->init_alpha;//ALPHA + irnd()%10 - 5;

	if ( ++work->id >= N_PRIMS ) work->id = 0;
    }
    

    work->cnt++;


    if ( work->name == 0 ) {

	if ( work->init_alpha <= 0.0f ) {
	    GV_DestroyActor( work );
	}

    }
    else {

	if ( RecieveKillMessage( work ) ) {
	    GV_DestroyActor( work );
	}

    }

    

}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float 			ftmp;
	float			angle;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	//DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, work->pos );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = COLOR * ( 0.20f*frnd() + 1.0f );
	    uvrgbwh->r  = ftmp;
	    uvrgbwh->g  = ftmp;
	    uvrgbwh->b  = ftmp;
	    uvrgbwh->a  = 0;

	    ftmp  = work->size * ( 1.20f + frnd() * 0.70f );
	    angle = TPI * frnd();
	    uvrgbwh->w  = ftmp * vu0_Cos( angle );
	    uvrgbwh->h  = ftmp * vu0_Sin( angle );

	    pos++;
	    uvrgbwh++;
	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



/* 注意！ -----------------------------------------------------------
  FVECTOR *pos には
  ゲーム中の呼び出しの場合,発生点のポインタが
  デモ呼び出しの場合,本体マトリクスからの相対シフト値が入っています。
--------------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR* pos, float size )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;


    if ( work->mat != NULL ) {		/* デモ */

	DG_COPY_VEC( &work->shift, pos );
	DG_SetPos( work->mat );
	DG_PutVector( &work->shift, &work->demo_pos, 1 );
	work->pos = &work->demo_pos;

    }else {				/* ゲーム */
	work->pos = pos;
    }

    work->size = size;
    work->initsize = size;
    work->rising_spd = RISING_SPEED * size / 100.f;
    work->id  = 0;

    work->life  = LIFE;
    work->cnt = 0;


    work->init_alpha = (float)ALPHA;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );

    /* fogで消すかどうかはデザイナーさんに要相談 */
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 10, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}



/*-------- void *NewHarrierDamageSmoke( FVECTOR* pos, float size, char *flag ) ----------
  
---------------------------------------------------------------------------------------*/  
void *NewHarrierDamageSmoke( FVECTOR* pos, float size, char *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->flag = flag;
		work->mat  = NULL;
		if ( GetResources( work, pos, size ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



/*-------- void *NewDemoHarrierDamageSmoke( FVECTOR* pos, float size, char *flag ) ----------
  
-------------------------------------------------------------------------------------------*/  
void *NewDemoHarrierDamageSmoke( int name, FMATRIX *mat, FVECTOR *shift, float size )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->flag = NULL;
		work->mat  = mat;
		if ( GetResources( work, shift, size ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



