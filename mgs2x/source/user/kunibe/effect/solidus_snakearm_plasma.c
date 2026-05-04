//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  solidus_snakearm_plasma.c
  ソリダス蛇手プラズマ
  2001/05/09 Yuuta Kunibe	
  $Id: solidus_snakearm_plasma.c,v 1.1.1.3 2002/11/19 11:44:51 Yoshizawa1 Exp $
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


#define COLOR			(64)
#define ALPHA			(64)//(64)

#define N_PRIMS			(3)
#define N_VERTS 		(14*4)
#define CENTER_NUM 		(N_PRIMS * N_VERTS * 2/2)

#define WIDTH			(2.0f)//(5.0f)
#define RANDOM_WIDTH		(200.0f)

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	NODE_NUM		(14)			/* 関節間数 */
#define NODE_POINT_NUM		(6)			/* 関節間のポイント数 */

#define	LEFT_START_NODE		(1)			/* 左プラズマ開始関節 */
#define	RIGHT_START_NODE	(17)			/* 右プラズマ開始関節 */




typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    DG_EVMOBJ	*evmobj;    

    FVECTOR	center[CENTER_NUM];
    FVECTOR	speed[CENTER_NUM];

    float	width_x[CENTER_NUM];
    float	width_y[CENTER_NUM];

	float	width;

    int		life;

} Work;


extern DG_EVMOBJ *SnakeArmEvmObj;


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

    FVECTOR		point;
    FVECTOR		add_vec;

    FVECTOR		to;
    
    FMATRIX		mat;
    SVECTOR		rot;
    FVECTOR		vecZ;


    if ( work->evmobj == NULL ) {		
		GV_DestroyActor( work );
		return;
    }

	// 2001.11.09修正
    if ( GM_CheckGameStatus( STATE_SCN_DEMO ) && SnakeArmEvmObj == NULL ) {
		printf("snakearm_null ------------------------ plasma_destroy \n");
		GV_DestroyActor( work );
		return;
    }

	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
		work->width = 2.0f;
	}
	else {
		if ( GM_PlayerStatus & PLAYER_WATCH ) {
			work->width = 2.0f;
		}
		else {
			work->width = 5.0f;
		}
	}

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



    /* 左蛇手 */

    to.vx = work->evmobj->def->skeleton[ LEFT_START_NODE ].rt_tx;
    to.vy = work->evmobj->def->skeleton[ LEFT_START_NODE ].rt_ty;
    to.vz = work->evmobj->def->skeleton[ LEFT_START_NODE ].rt_tz;
	
    /* 関節位置絶対化 */
    DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ LEFT_START_NODE ] );
    DG_PutVector( &to, &to, 1 ) ;
    DG_COPY_VEC( &work->center[0], &to );
    


    for ( i = 0 ; i < NODE_NUM ; i++ ) {	/* 蛇手１５関節 */

		/* 出発ノード算出 ->> 前ループでtoにノード座標が入っている */	
		DG_COPY_VEC( &point, &to );

		/* 目標ノード算出 */
        to.vx = work->evmobj->def->skeleton[ i+2 ].rt_tx;
		to.vy = work->evmobj->def->skeleton[ i+2 ].rt_ty;
		to.vz = work->evmobj->def->skeleton[ i+2 ].rt_tz;
	
		/* 関節位置絶対化 */
		DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ i+2 ] );
		DG_PutVector( &to, &to, 1 ) ;
	
		_sceVu0SubVector( &vecZ, &to, &point );
		_sceVu0ScaleVector( &add_vec, &vecZ, 0.16f );
		_sceVu0Normalize( &vecZ, &vecZ );
		OK_DirVecXY( &vecZ, &DG_ZeroVector, &rot );
    
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_GetPos( &mat );
	
		for ( j = 0 ; j < NODE_POINT_NUM ; j++ ) {	/* 関節間４ポイント */

			n = i * NODE_POINT_NUM + j;

			/* 最終点 */
			if ( n == N_PRIMS * N_VERTS / 2 - 1 ) {		
				_sceVu0SubVector( pos, &work->center[n], &width );
				pos++;
				_sceVu0AddVector( pos, &work->center[n], &width );
				pos++;
			}
			/* 繋ぎ目 */	
			else if ( i!=0 && !(n%28) ) {

				DG_COPY_VEC( &work->center[n+1], &work->center[n] );
				DG_COPY_VEC( &work->center[n], &work->center[n-1] );
				DG_COPY_VEC( pos, (pos-2) );
				pos++;
				DG_COPY_VEC( pos, (pos-2) );
				pos++;

			}
			/* 通常処理 */
			else {

				if ( n == N_PRIMS * N_VERTS / 2 - 2 ) {	/* 最終点はノード点へ */
					DG_COPY_VEC( &work->center[n+1], &to );
				}
				else {
					/* 基準点更新 */
					_sceVu0AddVector( &point, &point, &add_vec );

					/* ランダム幅算出 */
					_sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], work->width_x[n] );
					_sceVu0ScaleVector( &width,  (FVECTOR *)mat.m[1], work->width_y[n] );
					_sceVu0AddVector( &vectmp, &vectmp, &width );

					work->width_x[n] += work->speed[n].vx;
					work->width_y[n] += work->speed[n].vy;
					_sceVu0ScaleVector( &work->speed[n], &work->speed[n], 0.9f );
					work->width_x[n] *= 0.96f;
					work->width_y[n] *= 0.96f;
		    
					/* 次の中心算出 */
					_sceVu0AddVector( &work->center[n+1], &point, &vectmp );
				}

				/* 次の中心位置へのベクトルから幅ベクトル算出 */
				_sceVu0SubVector( &vectmp, &work->center[n+1], &work->center[n] );

				_sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)(&DG_Chanls[0].eye.m[2]) );
				_sceVu0Normalize( &vectmp, &vectmp );
				_sceVu0ScaleVector( &width, &vectmp, work->width );

				/* 頂点設定 */
				_sceVu0SubVector( pos, &work->center[n], &width );
				pos++;

				_sceVu0AddVector( pos, &work->center[n], &width );
				pos++;

			}	    
	    
		}

    }



    /* 右蛇手 */
    to.vx = work->evmobj->def->skeleton[ RIGHT_START_NODE ].rt_tx;
    to.vy = work->evmobj->def->skeleton[ RIGHT_START_NODE ].rt_ty;
    to.vz = work->evmobj->def->skeleton[ RIGHT_START_NODE ].rt_tz;
	
    /* 関節位置絶対化 */
    DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ RIGHT_START_NODE ] );
    DG_PutVector( &to, &to, 1 ) ;
    DG_COPY_VEC( &work->center[56], &to );
    


    for ( i = 0 ; i < NODE_NUM ; i++ ) {	/* 蛇手１５関節 */

		/* 出発ノード算出 ->> 前ループでtoにノード座標が入っている */	
		DG_COPY_VEC( &point, &to );

		/* 目標ノード算出 */
        to.vx = work->evmobj->def->skeleton[ i+18 ].rt_tx;
		to.vy = work->evmobj->def->skeleton[ i+18 ].rt_ty;
		to.vz = work->evmobj->def->skeleton[ i+18 ].rt_tz;
	
		/* 関節位置絶対化 */
		DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ i+18 ] );
		DG_PutVector( &to, &to, 1 ) ;
	
		_sceVu0SubVector( &vecZ, &to, &point );
		_sceVu0ScaleVector( &add_vec, &vecZ, 0.16f );
		_sceVu0Normalize( &vecZ, &vecZ );
		OK_DirVecXY( &vecZ, &DG_ZeroVector, &rot );
    
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_GetPos( &mat );
	
		for ( j = 0 ; j < NODE_POINT_NUM ; j++ ) {	/* 関節間４ポイント */

			n = 56 + i * NODE_POINT_NUM + j;

			/* 最終点 */
			if ( n == 56 + N_PRIMS * N_VERTS / 2 - 1 ) {		
				_sceVu0SubVector( pos, &work->center[n], &width );
				pos++;
				_sceVu0AddVector( pos, &work->center[n], &width );
				pos++;
			}
			/* 繋ぎ目 */	
			else if ( i!=0 && !(n%28) ) {

				DG_COPY_VEC( &work->center[n+1], &work->center[n] );
				DG_COPY_VEC( &work->center[n], &work->center[n-1] );
				DG_COPY_VEC( pos, (pos-2) );
				pos++;
				DG_COPY_VEC( pos, (pos-2) );
				pos++;

			}
			/* 通常処理 */
			else {

				if ( n == 56 + N_PRIMS * N_VERTS / 2 - 2 ) {	/* 最終点はノード点へ */
					DG_COPY_VEC( &work->center[n+1], &to );
				}
				else {
					/* 基準点更新 */
					_sceVu0AddVector( &point, &point, &add_vec );

					/* ランダム幅算出 */
					_sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], work->width_x[n] );
					_sceVu0ScaleVector( &width,  (FVECTOR *)mat.m[1], work->width_y[n] );
					_sceVu0AddVector( &vectmp, &vectmp, &width );

					work->width_x[n] += work->speed[n].vx;
					work->width_y[n] += work->speed[n].vy;
					_sceVu0ScaleVector( &work->speed[n], &work->speed[n], 0.9f );
					work->width_x[n] *= 0.96f;
					work->width_y[n] *= 0.96f;
		    
					/* 次の中心算出 */
					_sceVu0AddVector( &work->center[n+1], &point, &vectmp );
				}

				/* 次の中心位置へのベクトルから幅ベクトル算出 */
				_sceVu0SubVector( &vectmp, &work->center[n+1], &work->center[n] );

				_sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)(&DG_Chanls[0].eye.m[2]) );
				_sceVu0Normalize( &vectmp, &vectmp );
				_sceVu0ScaleVector( &width, &vectmp, WIDTH );

				/* 頂点設定 */
				_sceVu0SubVector( pos, &work->center[n], &width );
				pos++;

				_sceVu0AddVector( pos, &work->center[n], &width );
				pos++;

			}
	    
	    
		}

    }



    if ( --work->life <= 0 ) {
		GV_DestroyActor( work );
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, FVECTOR *color, DG_TEX *tex )
{

	int		i,j;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGB	*uvrgb;
	SVECTOR		rot;
	FMATRIX		mat;
	float		radius;
	int		flag;

	prim->raise = 0;

	rot.vx = 0;
	rot.vy = 0;
	rot.vz = irnd()%4096;

	flag = irnd()%2;
	
	//DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;	

	radius = RANDOM_WIDTH/2.f;
	
	for ( i = 0 ; i < N_PRIMS * 2; i++ ) {

	    for ( j = 0 ; j < N_VERTS ; j++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );

		pos++;

		if ( j%2 ) {
		    uvrgb->u = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		}
		else {
		    uvrgb->u = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		}

		/*if ( (j/2)%2 ) {
		    uvrgb->v = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		}
		else {
		    uvrgb->v = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		}*/
		//uvrgb->v = FTOI12( 1.0f / (float)CENTER_NUM * (float)(i*28+j/2) * tex->v_scale + tex->v_offset );
		uvrgb->v = FTOI12( 0.5f * tex->v_scale + tex->v_offset );

		uvrgb->q = 4096;
		
      //BP: first two verts have no kick bit set. NOT just the first, that causes a bad triangle from vert 0 to vert 1 and 2 (likely not noticable on PS2)
      if ( j < 2 )
      {
		    uvrgb->f = 0x8fff;
		}
		else 
      {
		    uvrgb->f = 0x0fff;
		}
		uvrgb->r = (int)color->vx;//COLOR;
		uvrgb->g = (int)color->vy;//COLOR;
		uvrgb->b = (int)color->vz;//COLOR;
		uvrgb->a = ALPHA;

		uvrgb++;

		if ( j % 2 ) {
		    DG_COPY_VEC( &work->center[i*28+j/2], &DG_ZeroVector );

		    if (flag) {
			rot.vz += 32 + irnd()%196;
		    }
		    else {
			rot.vz -= 32 + irnd()%196;
		    }

		    if ( rot.vz > 4096 ) rot.vz -= 4096;
		    else if ( rot.vz < 0 ) rot.vz += 4096;

		    DG_SetPos( &DG_UnitMatrix );
		    DG_RotatePos( &rot );
		    DG_GetPos( &mat );

		    radius += frnd()*RANDOM_WIDTH/40.f;
		    if ( radius > RANDOM_WIDTH ) {
			radius = RANDOM_WIDTH / 2.f;
		    }
		    else if ( radius < -RANDOM_WIDTH ) {
			radius = -RANDOM_WIDTH / 2.f;
		    }

		    work->width_x[i*28+j/2] = mat.m[0][0] * radius;
		    work->width_y[i*28+j/2] = mat.m[0][1] * radius;
		    		    
		    work->speed[i*28+j/2].vx = frnd()*RANDOM_WIDTH*0.025f;
		    work->speed[i*28+j/2].vy = frnd()*RANDOM_WIDTH*0.025f;
		    work->speed[i*28+j/2].vz = frnd()*RANDOM_WIDTH*0.025f;

		}

	    }
	    
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS * 2 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS * 2 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS * 2 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS * 2 );

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, DG_EVMOBJ *evmobj, FVECTOR *color, int life )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    
    work->evmobj = evmobj;

    work->life = life;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_SHADE,// | DG_PRIM2_TEX,
				      N_PRIMS * 2, N_VERTS );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "smoke_lp1_alp" ) );
    //tex = DG_GetTexture( GV_StrCode( "svc_bonbori_g" ) );
    InitPacket( work, prim, color, tex );
    
    return 0;

}



void *NewSolidusSnakearmPlasma( DG_EVMOBJ *evmobj, FVECTOR *color, int life )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, evmobj, color, life ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



