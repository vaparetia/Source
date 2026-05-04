//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_missile_smoke.c
  ハリアーミサイル弾軌跡雲
  2001/04/08 Yuuta Kunibe	
  $Id: harrier_missile_smoke.c,v 1.1.1.3 2002/11/19 11:44:42 Yoshizawa1 Exp $
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




#define N_PRIMS		(2)
#define N_VERTS 	(64)
#define CENTER_NUM 	(N_PRIMS * N_VERTS/2)

#define COLOR		(128)
#define ALPHA		(64)

#define SIZE		(100.f)
#define	SPEED		(100.f)

#define SCR_POS 	SCRPAD_ADDR
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define LIFE		(300)



/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR*	pos;
    FVECTOR*	vec;

    FVECTOR	center[CENTER_NUM];	/* 中心履歴 */
    FVECTOR	speed[CENTER_NUM];	/* 中心スピード */
    float	size[CENTER_NUM];	/* 幅 */
    float	u[N_PRIMS*N_VERTS];	/* テクスチャu値 */

    /* テクスチャスクロールの各パラメータ */
    float	u_scale;
    float	u_offset;
    float	add_u;
    
    int 	id;
    FVECTOR	wind;

    float	param;

    int		life;

    char 	alpha_flag;

    char	off_flag;
    char	*flag;

    int 	hit_id;
    
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
    FVECTOR		vecWidth;
    int 		alpha;



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



    if ( *work->flag ) {
	work->off_flag = 1;
	work->hit_id = work->id-1;
    }
    
    if ( work->off_flag ) {
	work->param -= 0.025f;
	if ( work->param < 0.0f ) {
	    work->param = 0.0f;
	}
    }
    else {
    
	if ( work->id == 0 ) {
	    DG_COPY_VEC( &work->center[work->id], &work->center[CENTER_NUM-1] );
	    DG_COPY_VEC( &work->speed[work->id], &work->speed[CENTER_NUM-1] );
	    work->size[work->id] = work->size[CENTER_NUM-1];

	    work->id = 1;

	    DG_COPY_VEC( &work->center[work->id], work->pos );
	    _sceVu0ScaleVector( &work->speed[work->id], work->vec, SPEED );
	    work->size[work->id] = SIZE;
	}
	else if ( work->id == 32 ) {
	    DG_COPY_VEC( &work->center[32], &work->center[31] );
	    DG_COPY_VEC( &work->speed[32], &work->speed[31] );
	    work->size[32] = work->size[31];

	    work->id = 33;

	    DG_COPY_VEC( &work->center[work->id], work->pos );
	    _sceVu0ScaleVector( &work->speed[work->id], work->vec, SPEED );
	    work->size[work->id] = SIZE;
	}
	else {
	    DG_COPY_VEC( &work->center[work->id], work->pos );
	    _sceVu0ScaleVector( &work->speed[work->id], work->vec, SPEED );
	    work->size[work->id] = SIZE;
	}

    }


    
#if 0    
    printf("id : %d\n",work->id);
#endif    


    if ( work->param > 0.f ) {
	
	/* ロケットミサイルは直線運動なので幅ベクトル計算は一回だけする */
	_sceVu0OuterProduct( &vecWidth, work->vec, (FVECTOR *)(&DG_Chanls[0].eye.m[2]) );
	_sceVu0Normalize( &vecWidth, &vecWidth );

	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    for ( j = 0 ; j < N_VERTS/2 ; j++ ) {

		n = i*32+j;
	    
		if ( n == 0 ) {

		    /*_sceVu0SubVector( &vectmp, &work->center[0], &work->center[CENTER_NUM-2] );
		      _sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)(&DG_Chanls[0].eye.m[2]) );
		      _sceVu0Normalize( &vectmp, &vectmp );
		      _sceVu0ScaleVector( &vectmp, &vectmp, work->size[0] );*/
		    _sceVu0ScaleVector( &vectmp, &vecWidth, work->size[0] );
		
		    /*_sceVu0SubVector( pos, &work->center[0], &vectmp );
		    pos++;*/

		    _sceVu0AddVector( pos, &work->center[0], &vectmp );
		    pos++;
		    _sceVu0SubVector( pos, &work->center[0], &vectmp );
		    pos++;

		}
		else if ( j == 0 ) {

		    DG_COPY_VEC( pos, (pos-2) );
		    pos++;
		    DG_COPY_VEC( pos, (pos-2) );
		    pos++;

		}
		else {

		    /*_sceVu0SubVector( &vectmp, &work->center[n], &work->center[n-1] );
		      _sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)(&DG_Chanls[0].eye.m[2]) );
		      if ( n == work->id + 1 ) {
		      _sceVu0SubVector( &vectmp, &DG_ZeroVector, &vectmp );
		      }
		      _sceVu0Normalize( &vectmp, &vectmp );
		      _sceVu0ScaleVector( &vectmp, &vectmp, work->size[n] );*/

		    _sceVu0ScaleVector( &vectmp, &vecWidth, work->size[n] );
		
		    _sceVu0AddVector( pos, &work->center[n], &vectmp );
		    pos++;
		    _sceVu0SubVector( pos, &work->center[n], &vectmp );
		    pos++;

		}

		/* 問題あり */
		if ( !work->alpha_flag ) {
		    alpha = 64 * n / ( work->id + 1 );
		    alpha = alpha / 2 * work->param;
		}
		else {
		    alpha = work->id - n;
		    if ( alpha < 0 ) alpha += 63;
		    alpha = 63 - alpha;
		    alpha = alpha / 2;
		    alpha = (int)( (float)alpha * work->param );
		}
	    
		if ( work->id == 62 ) {
		    if ( n == 63 || n == 0 || n == 1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		else if ( work->id == 63 ) {
		    if ( n == 0 || n == 1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		else if ( work->id == 31 ) {
		    if ( n == 32 || n == 33 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		else {		    		    
		    if ( n == work->id+1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}

		if ( work->off_flag && n == work->hit_id ) {
		}
		else {
		    _sceVu0SubVector ( &work->center[n], &work->center[n], &work->speed[n] );
		    _sceVu0ScaleVector( &work->speed[n], &work->speed[n], 0.99f );
		}
	    
	    }

	}


    }


    if ( !work->off_flag ) {

	if ( ++work->id >= CENTER_NUM ) {
	    work->alpha_flag = 1;
	    work->id   = 0;
	}

    }


    if ( --work->life == 0 ) {
	GV_DestroyActor( work );
    }
    else if ( work->param <= 0.f ) {
	GV_DestroyActor( work );
    }
	

}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
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

		DG_COPY_VEC( pos, work->pos );

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
		uvrgb->b = COLOR;
		uvrgb->r = COLOR;
		uvrgb->g = COLOR;
		uvrgb->a = ALPHA;

		uvrgb++;

		if ( j % 2 ) {
		    DG_COPY_VEC( &work->center[i*32+j/2], work->pos );
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



static int GetResources( Work *work, FVECTOR *pos, FVECTOR *vec, char *flag )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    
    work->pos        = pos;
    work->vec        = vec;
    work->param      = 1.0f;
    work->life       = DIRECT_TICK(LIFE);
    work->alpha_flag = 0;
    work->off_flag   = 0;
    work->flag       = flag;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_TEX,
				      N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "smoke_lp3_alp" ) );
    InitPacket( work, prim, tex );
    
    return 0;

}



/*
  NewHarrierMissileSmokeLine : 呼び出し関数
  FVECTOR *pos : 発生位置
  FVECTOR *vec : 移動方向  
*/  
void *NewHarrierMissileSmokeLine( FVECTOR *pos, FVECTOR *vec, char *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, vec, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


