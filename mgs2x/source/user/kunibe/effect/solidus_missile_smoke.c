//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_missile_smoke.c
    ソリダスミサイル煙
    2001/05/11 Yuuta Kunibe	
    $Id: solidus_missile_smoke.c,v 1.1.1.3 2002/11/19 11:44:50 Yoshizawa1 Exp $
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


#define N_PRIMS	(12)
#define	N_VERTS	(16)
#define	N_SPRTS	(N_PRIMS*N_VERTS)

#define COLOR	(128)//(192)
#define ALPHA	(30)//(60)

#define SPEED	(50.f)

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
    int		name;

    DG_PRIM2	*prim;

    FVECTOR*	pos;
    FVECTOR	hit_pos;

    FVECTOR	*vector;
    FVECTOR	hit_vec;

    FVECTOR	vec[N_SPRTS];
    float	radius[N_SPRTS];
    float	cos[N_SPRTS];
    float	sin[N_SPRTS];

    float	size;

    float	init_size;
    float	init_speed;
    int		init_alpha;

    FVECTOR	wind;

    int  	id;

    int 	*flag;
    int		demo_flag;
    int		off_flag;
    
} Work;


static void RecieveMessage( Work *work )
{
    GV_MSG*	msg;
    int 	n_msg;


    if ( work->name == 0 ) {
	return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:		/* 着弾 */
	    work->demo_flag = 1;
	    break;
	}
	msg++;
    }

    return;

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
    float		len;
    float		ftmp;
    float		angle;
    FVECTOR		vectmp;
    FVECTOR		prevpos;
    FVECTOR		unit_vec;
    int			alpha;

    FVECTOR		vector;




    RecieveMessage( work );

    

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
    /*intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;*/


    /* ミサイル衝突判定 */
    if ( *work->flag ) {
	if ( work->off_flag == 0 ) {
	    DG_COPY_VEC( &work->hit_pos, work->pos );
	    work->pos = &work->hit_pos;
	    DG_COPY_VEC( &work->hit_vec, work->vector );
	    work->vector = &work->hit_vec;
	    work->off_flag = 1;
	}
    }


    /* 直前の煙取得用ID */
    n = work->id - 1;
    if ( n < 0 ) n = N_SPRTS - 1;


    /* スプライト更新 */
    for ( i = 0 ; i < N_SPRTS ; i++ ) {	

	/* 位置更新 */
	_sceVu0SubVector( &pos[i], &pos_pre[i], &work->vec[i] );


	j = work->id - i;
	if ( j < 0 ) j += N_SPRTS;
	
	if ( j < 40 ) {
	    alpha = work->init_alpha * j / 40;
	}
	else {
	    alpha = work->init_alpha * ( N_SPRTS - j  ) / ( N_SPRTS - 40 );
	}
	
	uvrgbwh[i].a = alpha;
	
	
	if ( uvrgbwh[i].a > 0 ) {
	    ftmp = work->radius[i] * (float)( N_SPRTS - j ) / (float)N_SPRTS + work->radius[i] * 1.5f * (float)j / (float)N_SPRTS;
	    work->radius[i] *= 1.02f;
	    uvrgbwh[i].w = ftmp * work->cos[i];
	    uvrgbwh[i].h = ftmp * work->sin[i];
	}
	else {
	    uvrgbwh[i].w = 0;
	    uvrgbwh[i].h = 0;
	}

	if ( i == n ) DG_COPY_VEC( &prevpos, &pos[i] );

    }




    /* 前の煙との直線距離から発生させる煙の数を算出 */
    /*if ( work->off_flag ) {
	DG_COPY_VEC( &vectmp, &work->hit_pos );
    }
    else {*/
	DG_COPY_VEC( &vectmp, work->pos );
    //}
    vectmp.vx += 200.f * frnd();
    vectmp.vy += 200.f * frnd();
    vectmp.vz += 200.f * frnd();
    len = GV_VecLen3F2( &prevpos, &vectmp );


    n = (int)( len / ( work->size / 3.f ) );
    n += 1;
    ftmp = 1.0f / (float)n;
    _sceVu0SubVector( &unit_vec, &vectmp, &prevpos );
    _sceVu0ScaleVector( &unit_vec, &unit_vec, ftmp );

    
    work->init_size = work->init_size * ( 1.f + frnd()*0.1f );

    if ( !(irnd()%120) ) work->init_size = work->size * 1.25f;
    
    if ( work->init_size > work->size * 1.25f ) {
	work->init_size = work->size * 1.25f;
    }
    else if ( work->init_size < work->size * 0.8f ) {
	work->init_size = work->size * 0.8f;
    }

    

    _sceVu0Normalize( &vector, work->vector );

    /* 算出した個数分煙生成 */
    for ( i = 0 ; i < n ; i++ ) {

	DG_COPY_VEC( &pos[work->id], &prevpos );

	_sceVu0ScaleVector( &vectmp, &unit_vec, (float)(i+1) );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &vectmp );
	pos[work->id].vx += work->size / 4.f * frnd();
	pos[work->id].vy += work->size / 4.f * frnd();
	pos[work->id].vz += work->size / 4.f * frnd();

	//_sceVu0ScaleVector( &work->vec[work->id], work->vector, work->init_speed );
	_sceVu0ScaleVector( &work->vec[work->id], &vector, work->init_speed );
	work->vec[work->id].vx += frnd()*10.f;
	work->vec[work->id].vy += frnd()*10.f;
	work->vec[work->id].vz += frnd()*10.f;

	work->radius[work->id] = work->init_size * ( 1.2f + frnd() * 0.40f );
	angle = TPI * frnd();
	work->cos[work->id] = vu0_Cos( angle );
	work->sin[work->id] = vu0_Sin( angle );
	uvrgbwh[work->id].w  = work->radius[work->id] * work->cos[work->id];
	uvrgbwh[work->id].h  = work->radius[work->id] * work->sin[work->id];
	
	if ( ++work->id >= N_SPRTS ) work->id = 0;

    }


    /* 衝突後のフェードアウト */
    if ( work->off_flag ) {
	if ( work->init_alpha > 0 ) {
	    work->init_alpha -= 1;
	    work->init_speed *= 0.9f;
	}
    }

    
    if ( work->init_alpha <= 0 ) {
	GV_DestroyActor( work );
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

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_SPRTS ; i++ ) {

	    DG_COPY_VEC( pos, work->pos );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = COLOR * ( frnd()* 0.2f + 0.8f );
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

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR* pos, FVECTOR *vec, float size )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;


    work->pos = pos;
    work->vector = vec;
    work->size = size;
    work->init_size  = size;
    work->init_speed = SPEED;
    work->init_alpha = ALPHA;

    work->id  = 0;
    work->off_flag = 0;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}

/*
  NewRisingSmoke
      FVECTOR* pos : 発生位置
*/
void *NewSolidusMissileSmoke( FVECTOR* pos, FVECTOR *vec, float size, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->flag = flag;
		if ( GetResources( work, pos, vec, size ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



/* デモ用呼びだし口 */
void *NewDemoSolidusMissileSmoke( int name, FMATRIX *mat )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->demo_flag = 0;
		work->flag = &work->demo_flag;
		if ( GetResources( work, (FVECTOR *)mat->m[3], (FVECTOR *)mat->m[1], 200.0f ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}

