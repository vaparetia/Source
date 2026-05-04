//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  damage_plasma.c
  ダメージプラズマ
  2001/06/04 Yuuta Kunibe	
  $Id: damage_plasma.c,v 1.1.1.3 2002/11/19 11:44:36 Yoshizawa1 Exp $
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


#define COLOR			(128)
#define ALPHA			(64)

#define N_PRIMS			(64)
#define N_VERTS 		(32)
#define	SPRT_NUM		(N_PRIMS*N_VERTS)

#define WIDTH			(10.0f)
#define RANDOM_WIDTH		(300.0f)

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	NODE_NUM		(14)			/* 関節間数 */
#define NODE_POINT_NUM		(6)			/* 関節間のポイント数 */

#define	LEFT_START_NODE		(1)			/* 左プラズマ開始関節 */
#define	RIGHT_START_NODE	(17)			/* 右プラズマ開始関節 */

#define LIFE			(30)			/* 寿命 */


#define	SPEED			(50.0f)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    OBJECT	*object;

    FVECTOR	center[SPRT_NUM];
    FVECTOR	speed[SPRT_NUM];

    float	width_x[SPRT_NUM];
    float	width_y[SPRT_NUM];

    int		life;

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

    FVECTOR		point;
    FVECTOR		add_vec;

    FVECTOR		to;
    
    FMATRIX		mat;
    SVECTOR		rot;
    FVECTOR		vecZ;

    static int		node_point[5] = { 15, 13, 1, 7, 9 };




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



    DG_COPY_VEC( &to, work->object->objs->objs[node_point[0]].world.m[3] );
    DG_COPY_VEC( &point, &to );


    for ( i = 0 ; i < 4 ; i++ ) {	/* 蛇手１５関節 */

	/* 出発ノード算出 ->> 前ループでtoにノード座標が入っている */	
	DG_COPY_VEC( &point, &to );


	DG_COPY_VEC( &to, work->object->objs->objs[node_point[i+1]].world.m[3] );
	
	_sceVu0SubVector( &vecZ, &to, &point );
	_sceVu0ScaleVector( &add_vec, &vecZ, ( 1.0f / (float)N_VERTS / 16.0f ) );
	_sceVu0Normalize( &vecZ, &vecZ );
	OK_DirVecXY( &vecZ, &DG_ZeroVector, &rot );
    
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_GetPos( &mat );

	
	for ( j = 0 ; j < N_VERTS*16 ; j++ ) {

	    n = i * N_VERTS*16 + j;

	    /* 基準点更新 */
	    _sceVu0AddVector( &point, &point, &add_vec );

	    /* ランダム幅算出 */
	    /*_sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], work->width_x[n] );
	    _sceVu0ScaleVector( &width,  (FVECTOR *)mat.m[1], work->width_y[n] );
	    _sceVu0AddVector( &vectmp, &vectmp, &width );*/

	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], work->center[n].vx );

	    /*work->width_x[n] += work->speed[n].vx;
	    work->width_y[n] += work->speed[n].vy;
	    _sceVu0ScaleVector( &work->speed[n], &work->speed[n], 0.9f );

	    work->width_x[n] *= 0.96f;
	    work->width_y[n] *= 0.96f;*/

	    _sceVu0AddVector( pos, &point, &vectmp );

	    pos++;

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

	int			i,j,k,l,n;
	FVECTOR			*pos;	
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	SVECTOR			rot;
	FMATRIX			mat;
	float			radius;
	int			flag;
	float			param[3];
	float			add_param[3];
	int			param_flag[3];




	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = 0;


	rot.vx = 0;
	rot.vy = 0;
	rot.vz = irnd()%4096;

	if (irnd()%2) {
	    flag = 1;
	}
	else {
	    flag = 0;
	}

	
	pos   	= SCR_POS;
	uvrgbwh	= SCR_UVS;	

	
	radius = RANDOM_WIDTH/2.f;


	param[0] = 0.f;
	param[1] = 0.f;
	param[2] = 0.f;
	add_param[0] = 0.f;
	add_param[1] = 0.f;
	add_param[2] = 0.f;
	param_flag[0] = 0;
	param_flag[1] = 0;
	param_flag[2] = 0;
	


	for ( l = 0 ; l < 8 ; l++ ) {

	    pos   	= SCR_POS;
	    uvrgbwh	= SCR_UVS;	

	    for ( i = 0 ; i < N_PRIMS/8; i++ ) {

		for ( k = 0 ; k < 3 ; k++ ) {
		    if ( param_flag[k] == 1 ) {
			add_param[k] =-0.036f - frnd()*0.004f;
			if ( param[k] + add_param[k] < 0.25f ) {
			    param_flag[k] = 0;
			}
		    }
		    else if ( param_flag[k] == -1 ) {
			add_param[k] = 0.036f + frnd()*0.004f;
			if ( param[k] + add_param[k] > -0.25f ) {
			    param_flag[k] = 0;
			}
		    }
		    else {			   			    
			add_param[k] = frnd()*0.04f;
			if ( param[k] + add_param[k] > 0.30f ) {
			    param_flag[k] = 1;
			}
			else if ( param[k] + add_param[k] < -0.30f ) {
			    param_flag[k] = -1;
			}
		    }

		    add_param[k] *= 1.f/32.f;

		}       


		for ( j = 0 ; j < N_VERTS ; j++ ) {

		    DG_COPY_VEC( pos, &DG_ZeroVector );
		    
		    pos++;

		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->q0 = 4096;
		    uvrgbwh->f0 = 0x0fff;

		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->q1 = 4096;
		    uvrgbwh->f1 = 0x0fff;

		    uvrgbwh->w = 20;
		    uvrgbwh->h = 20;

		    uvrgbwh->r = 32;
		    uvrgbwh->g = 64;
		    uvrgbwh->b = 255;
		    uvrgbwh->a = 64;

		    uvrgbwh++;



		    n = l * N_PRIMS / 8 * N_VERTS + i * N_VERTS + j;

		    DG_COPY_VEC( &work->center[n], &DG_ZeroVector );

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

		    work->width_x[n] = mat.m[0][0] * radius;
		    work->width_y[n] = mat.m[0][1] * radius;

		    work->speed[n].vx = SPEED * param[0];
		    work->speed[n].vy = SPEED * param[1];
		    work->speed[n].vz = SPEED * param[2];

		    param[0] += add_param[0];
		    param[1] += add_param[1];
		    param[2] += add_param[2];		    		    

		    work->center[n].vx = 100.0f + radius;

		}

	    
	    }

	    OK_Scr_Mem( &prim->pos[ 0 ][l*N_PRIMS/8*N_VERTS], SCR_POS, sizeof(FVECTOR), N_PRIMS/8*N_VERTS );
	    OK_Scr_Mem( &prim->pos[ 1 ][l*N_PRIMS/8*N_VERTS], SCR_POS, sizeof(FVECTOR), N_PRIMS/8*N_VERTS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 0 ]))[l*N_PRIMS/8*N_VERTS], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/8*N_VERTS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 1 ]))[l*N_PRIMS/8*N_VERTS], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/8*N_VERTS );

	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, OBJECT *object )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    
    work->object = object;

    work->life       = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT | DG_PRIM2_ALPHA | DG_PRIM2_SHADE | DG_PRIM2_TEX,
				      N_PRIMS, N_VERTS );

    //tex = DG_GetTexture( GV_StrCode( "smoke_lp3_alp" ) );
    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex );
    
    return 0;

}



/*
  NewSolidusDamagePlasma : 呼び出し関数
  FMATRIX *mat_parent
*/  
void *NewDamagePlasma( OBJECT *object )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, object ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



