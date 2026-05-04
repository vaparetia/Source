//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  solidus_damage_plasma.c
  2001/05/09 Yuuta Kunibe	
  $Id: solidus_damage_plasma.c,v 1.1.1.3 2002/11/19 11:44:49 Yoshizawa1 Exp $
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


#define COLOR		(128)
#define ALPHA		(64)

#define N_PRIMS		(2)
#define N_VERTS 	(64)
#define CENTER_NUM 	(N_PRIMS * N_VERTS/2)

#define WIDTH		(5.0f)
#define	POINT_LENGTH	(200.0f)
#define RANDOM_WIDTH	(20.f)//(50.0f)

#define SIZE		(100.f)
#define	SPEED		(100.f)

#define SCR_POS 	SCRPAD_ADDR
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define LIFE		(60)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR*	pos;			/* 発生位置 */
    FVECTOR*	vec;

    FVECTOR	center[CENTER_NUM];	/* 中心履歴 */
    FVECTOR	speed[CENTER_NUM];	/* 中心スピード */
    float	size[CENTER_NUM];	/* 幅 */
    float	u[N_PRIMS*N_VERTS];	/* テクスチャu値 */

    FVECTOR	wind;

    float	param;

    int		life;

    int 	hit_id;

    SVECTOR	rot_base;
    SVECTOR	rot_add;

    FMATRIX	*mat_parent;
    float	radius;
    
} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int 		i,j,n;
    int	  		clock;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;
    FVECTOR		vectmp;
    FVECTOR		width;

    FMATRIX		mat_base;
    FMATRIX		mat;
    SVECTOR		rot;


    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = work->prim->buffer_clock;
    pos	      = work->prim->pos[clock];
    pos_pre   = work->prim->pos[1-clock];
    uvrgb     = work->prim->uvrgb[clock];
    uvrgb_pre = work->prim->uvrgb[1-clock];

    /* ベースマトリクス取得 */
    work->rot_base.vx += work->rot_add.vx;
    work->rot_base.vz += work->rot_add.vz;

    DG_SetPos( work->mat_parent );
    DG_RotatePos( &work->rot_base );
    DG_GetPos( &mat_base );

    /* 半径取得 */
    if ( work->life > LIFE / 2 ) {
	work->radius -= 2.5f;
    }

    /* 回転角初期化 */
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0;

    /* 各点算出 */
    _sceVu0ScaleVector( &vectmp, (FVECTOR *)mat_base.m[0], work->radius );
    _sceVu0AddVector( &work->center[0], ( FVECTOR * )work->mat_parent->m[3], &vectmp );
    
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	for ( j = 0 ; j < N_VERTS/2 ; j++ ) {

	    n = i*32+j;

	    /* 最終点 */
	    if ( n == N_PRIMS * N_VERTS / 2 ) {		
		_sceVu0SubVector( pos, &work->center[n], &width );
		pos++;
		_sceVu0AddVector( pos, &work->center[n], &width );
		pos++;
	    }
	    /* 繋ぎ目 */	
	    else if ( i != 0 && j == 0 ) {

		DG_COPY_VEC( &work->center[n+1], &work->center[n] );
		DG_COPY_VEC( &work->center[n], &work->center[n-1] );
		DG_COPY_VEC( pos, (pos-2) );
		pos++;
		DG_COPY_VEC( pos, (pos-2) );
		pos++;

	    }
	    /* 通常処理 */
	    else {

		rot.vy += 4096 / ( N_PRIMS * N_VERTS / 2 - 1 );

		DG_SetPos( &mat_base );
		DG_RotatePos( &rot );
		DG_GetPos( &mat );
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], work->radius );
		vectmp.vy *= 1.50f;
		_sceVu0AddVector( &work->center[n+1], &vectmp, ( FVECTOR * )work->mat_parent->m[3] );
		work->center[n+1].vx += RANDOM_WIDTH * frnd();
		work->center[n+1].vy += RANDOM_WIDTH * frnd();
		work->center[n+1].vz += RANDOM_WIDTH * frnd();	       		

		_sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)(&DG_Chanls[0].eye.m[2]) );
		_sceVu0Normalize( &vectmp, &vectmp );
		_sceVu0ScaleVector( &width, &vectmp, WIDTH );

		_sceVu0SubVector( pos, &work->center[n], &width );
		pos++;

		_sceVu0AddVector( pos, &work->center[n], &width );
		pos++;

	    }
	    
	    
	}

    }



    if ( ++work->life >= LIFE ) {
	GV_DestroyActor( work );
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int		i,j;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGB	*uvrgb;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;

	for ( i = 0 ; i < N_PRIMS; i++ ) {

	    for ( j = 0 ; j < N_VERTS ; j++ ) {

		DG_COPY_VEC( pos, ( FVECTOR * )work->mat_parent->m[3] );

		pos++;

		if ( j%2 ) {
		    uvrgb->u = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		}
		else {
		    uvrgb->u = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		}

		if ( (j/2)%2 ) {
		    uvrgb->v = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		}
		else {
		    uvrgb->v = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		}
		//uvrgb->v = FTOI12( 1.0f / 64.f * (float)(i*32+j/2) * tex->v_scale + tex->v_offset );

		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = COLOR;
		uvrgb->g = COLOR;
		uvrgb->b = COLOR;
		uvrgb->a = ALPHA;

		uvrgb++;

		if ( j % 2 ) {
		    DG_COPY_VEC( &work->center[i*32+j/2], ( FVECTOR * )work->mat_parent->m[3] );
		    DG_COPY_VEC( &work->speed[i*32+j/2], &DG_ZeroVector );

		}

	    }
	    
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FMATRIX *mat_parent )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    
    work->life       = 0;

    work->mat_parent = mat_parent;

    work->rot_base.vx = irnd()%512;
    work->rot_base.vy = 0;
    work->rot_base.vz = irnd()%2048;

    work->rot_add.vx = irnd()%41-20;
    work->rot_add.vy = 0;
    work->rot_add.vz = irnd()%41-20;

    work->radius = 250.0f + frnd() * 100.0f;
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_TEX,
				      N_PRIMS, N_VERTS );

    tex = DG_GetTexture( GV_StrCode( "smoke_lp3_alp" ) );
    InitPacket( work, prim, tex );
    
    return 0;

}



/*
  NewSolidusDamagePlasma : 呼び出し関数
  FMATRIX *mat_parent
*/  
void *NewSolidusDamagePlasma( FMATRIX *mat_parent )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, mat_parent ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



