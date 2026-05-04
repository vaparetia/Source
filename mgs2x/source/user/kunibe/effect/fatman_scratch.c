//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  fatman_scratch.c
  ファットマン足傷

  2001/06/26 Yuuta Kunibe	
  $Id: fatman_scratch.c,v 1.1.1.3 2002/11/19 11:44:39 Yoshizawa1 Exp $
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



#define N_PRIMS			(16)
#define N_VERTS 		(64)
#define CENTER_NUM 		(N_PRIMS * N_VERTS/2)

#define COLOR			(48)
#define ALPHA			(128)

#define SCR_POS 		SCRPAD_ADDR
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

/* 床判定情報 */
#define HALF_SEARCH_LENGTH	(400.0f)	/* 床検索幅(半分) */
#define	FLOOR_OFFSET		(10.0f)		/* 床からの浮かせ */

/* 足傷情報 */
#define	SCRATCH_NUM		(2)

#define	LEFT_FOOT		(0)
#define	RIGHT_FOOT		(1)

#define	SCRATCH_OFF		(0)
#define	SCRATCH_ON		(1)
#define	SCRATCH_OFFSET		(100.f)

#define SCRATCH_WIDTH		(20.f)

#define LEFT_FOOT_NODE		(20)
#define	RIGHT_FOOT_NODE		(16)

#define	SPARK_SPEED		(100.0f)//(50.0f)

#define LIFE			(200)


extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count ) ;

extern void *NewFatmanScratchSpark(
		int	  n_prims,
		FMATRIX	  *world,
		FMATRIX	  *node_mat,
		float	  min_speed,
		float	  speed_wide,
		float	  gravity,
		SVECTOR	  *rot,
		SVECTOR	  *rot_wide,
		FVECTOR	  *color,
		float	  length,
		int	  count );

void *NewFatmanScratchSmoke(
	FMATRIX *mat,			// 発生マトリクス
	FVECTOR	*vec,
	int 	*flag
);


enum {
    GAME_SCRATCH,
    DEMO_SCRATCH,
};


typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int		mode;

    CONTROL	*control;
    OBJECT	*object;
    FMATRIX	*foot_node[SCRATCH_NUM];

    DG_PRIM2	*prim[SCRATCH_NUM];

    FVECTOR	center[SCRATCH_NUM][CENTER_NUM];
    FVECTOR	width[SCRATCH_NUM][CENTER_NUM];
    int		kick_flag[SCRATCH_NUM][CENTER_NUM];

    int 	id[SCRATCH_NUM];
    int		pre_scratch[SCRATCH_NUM];
    FVECTOR	pre_pos[SCRATCH_NUM];
    FVECTOR	move[SCRATCH_NUM];
    
    int		*flag;
    int		pre_flag;
    int		demo_flag;

    int		life;

} Work;




static inline float _Vu0VecLenXYZ( FVECTOR *vec )
{
	float	ans;

#ifdef BP_PSX2_ASM	 /* yano 2002.03.13 */
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    vmul.xyz    vf5, vf4, vf4
    vmulax.w	ACC, vf0, vf5x
    vmadday.w	ACC, vf0, vf5y
    vmaddz.w	vf5, vf0, vf5z
	vsqrt		Q, vf5w
	vwaitq
	vaddq.x		vf6, vf0, Q
    qmfc2.i		$8,vf6
    sw			$8,0(%0)
	": : "r"(&ans), "r"(vec) : "$8", "memory" );
#else
	ans = vec->vx*vec->vx + vec->vy*vec->vy + vec->vz*vec->vz;
	ans = DG_SQRT( ans );
#endif

	return (ans);
}


/*-------- SearchFloor : 床検索関数 ----------
       FMATRIX *floor_pos	: 床座標 (出力)
       FVECTOR *width		: 幅ベクトル(出力)	
       FMATRIX *mat		: 参照マトリクス(ファットマンの足ノードのマトリクス)
--------------------------------------------*/  
static int SearchFloor( FVECTOR *floor_pos, FVECTOR *normal, FVECTOR *width, FMATRIX *mat )
{

    HZX_FLR		seg;
    u_int		atr;
    int			hazard_flag;

    FVECTOR		from;
    FVECTOR		to;


    /* 検索始点終点設定 */
    DG_COPY_VEC( &from, (FVECTOR *)mat->m[3] );
    from.vy += HALF_SEARCH_LENGTH;
    DG_COPY_VEC( &to, (FVECTOR *)mat->m[3] );
    to.vy -= HALF_SEARCH_LENGTH;

    /* ハザードチェック */
    hazard_flag = HZX_OnlineHazardCheck( 
			    GM_GetHzxGroupID( GM_CurrentStageMap ),
			    &from,
			    &to,
			    HZX_CHK_FIX,
			    HZX_SEG_NO_RECOIL | HZX_SEG_RECOIL_TYPE,
			    HZX_FLOOR_NO_PLAYER );

    if ( hazard_flag == 2 ) {	/* 床検索成功 */

	/* 床情報取得 */
	HZX_GetOnlineHazard( &seg, &atr ) ;

	/* 床座標取得 */
	HZX_GetOnlinePoint( floor_pos );
	floor_pos->vy += FLOOR_OFFSET;

	/* 床法線取得 */
	normal->vx = seg.p1.h;
	normal->vy = seg.p3.h;
	normal->vz = seg.p2.h;
	_sceVu0Normalize( normal, normal );

	/* 幅ベクトル算出 */
	_sceVu0OuterProduct( width, normal, (FVECTOR *)mat->m[2] );

	return 1;

    }
    else {			/* 床検索失敗 */
	//printf("fatman_scratch.c L145 : fail_search_floor!\n");
	return 0;
    }

}



/* 仮火花 */
static void CallScratchSpark( FVECTOR *floor_pos, FVECTOR *normal, FVECTOR *move, float speed, FMATRIX *node_mat, int flag )
{

    FMATRIX	mat;
    FVECTOR	vec_x;
    FVECTOR 	color;
    SVECTOR	rot;
    SVECTOR	rot_wide;

    
    DG_COPY_VEC( (FVECTOR *)mat.m[1], normal );

    _sceVu0OuterProduct( &vec_x, normal, move );
    DG_COPY_VEC( (FVECTOR *)mat.m[0], &vec_x );

    _sceVu0OuterProduct( &vec_x, (FVECTOR *)mat.m[0], (FVECTOR *)mat.m[1] );
    DG_COPY_VEC( (FVECTOR *)mat.m[2], &vec_x );

    DG_COPY_VEC( (FVECTOR *)mat.m[3], floor_pos );
    mat.m[3][3] = 1.0f;
    
    //HZX_ViewMatrix( &mat, 1000.0f );


    rot.vx = 2048;
    rot.vy = -128;
    rot.vz = 0;

    rot_wide.vx = 256;
    rot_wide.vy = 256;
    rot_wide.vz = 1;

    color.vx = 255.0F;
    color.vy = 128.0F;
    color.vz = 32.0F;
    color.vw = 50.0F;    


    if ( flag ) {
	NewFatmanScratchSpark( 8,
			       &mat,
			       node_mat,
			       1.0f, speed, 0.0f,
			       &rot, &rot_wide,
			       &color, 1.5f+frnd()*0.50f, 4+irnd()%8 );
    }
    else {
	NewFatmanScratchSpark( 8,
			       &mat,
			       node_mat,
			       1.0f, speed, 0.0f,
			       &rot, &rot_wide,
			       &color, 0.8f+frnd()*0.2f, 4+irnd()%8 );
    }

}


/* デモ用火花メッセージ取得関数 */
static void ScratchRecieveMessage( Work* work )
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
	case 0:		
	    work->demo_flag = 1;
	    break;
	case 1:
	    work->demo_flag = 0;
	    break;
	}
	msg++;
    }

}



/* アクト関数 */
static void Act( Work *work )
{
    static int foot_node[2] = { LEFT_FOOT_NODE, RIGHT_FOOT_NODE };
    
    int			foot;

    int 		i,j,n;
    int	  		clock;
    DG_PRIM2		*prim;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;
    FVECTOR		*center;
    FVECTOR		*width;
    int			*kick_flag;

    FVECTOR		vectmp;
    FVECTOR		normal;
    FVECTOR		move;
    FMATRIX		node_mat;

    FVECTOR		width_tmp;
    FVECTOR		center_tmp;
    int			scratch;
    int			alpha;

    float		speed;
    int			spark_flag = 0;
    
    static FVECTOR	stage_center = { 56500.0f, 11500.0f, -95750.0f, 1.0f };
    int			check_flag;

    // デモ用火花メッセージ取得関数
    if ( work->mode == DEMO_SCRATCH )
    {
		ScratchRecieveMessage( work );
    }

    for ( foot = 0 ; foot < SCRATCH_NUM ; foot++ )
    {
      int buffSwitch;
		/* 操作する頂点バッファ取得 */
		prim = work->prim[foot];

      //AR_PARTICLE_FULL
      buffSwitch = DG_SwitchBuffPrim2( prim ) ;
		clock = prim->buffer_clock;

		pos	    = prim->pos[clock];
		pos_pre   = prim->pos[buffSwitch ^ clock];
		uvrgb     = prim->uvrgb[clock];
		uvrgb_pre = prim->uvrgb[buffSwitch ^ clock];

		center    = work->center[foot];
		width     = work->width[foot];
		kick_flag = work->kick_flag[foot];


		/* 足ノードのマトリクス取得 */
		if ( work->mode == DEMO_SCRATCH ) {	    
			DG_COPY_MAT( &node_mat, work->foot_node[foot] );
		}
		else {
			DG_COPY_MAT( &node_mat, &work->object->objs->objs[foot_node[foot]].world );
		}
		_sceVu0SubVector( &work->move[foot], (FVECTOR *)node_mat.m[3], &work->pre_pos[foot] );


		/* 床位置,幅ベクトル取得 */
		if ( !( SearchFloor( &center_tmp, &normal, &width_tmp, &node_mat ) ) ) {
			scratch = SCRATCH_OFF;
		}
		else {

			/* 地面との高さの差で足接地判定 */
			check_flag = 0;
			if ( node_mat.m[3][1] > center_tmp.vy + SCRATCH_OFFSET ) {
				check_flag = 0;
			}
			else {
				check_flag = 1;
			}

			/* 足傷エリア判定 */
			scratch = SCRATCH_OFF;
			if ( check_flag == 0 ) {
				scratch = SCRATCH_OFF;
			}
			else {

				if ( node_mat.m[3][0] <= 43000.0f || node_mat.m[3][0] >= 70000.0f
					 || node_mat.m[3][2] <= -112000.0f || node_mat.m[3][2] >= -79500.0f ) {	// 鉄板上 
					spark_flag = 1;
					scratch = SCRATCH_OFF;
				}
				else {

					_sceVu0SubVector( &vectmp, (FVECTOR *)node_mat.m[3], &stage_center );
					if ( abs(vectmp.vx) + abs(vectmp.vz) >= 25750.0f ) {			// 鉄板上
						spark_flag = 1;
						scratch = SCRATCH_OFF;
					}
					else {									// フロア上
						scratch = SCRATCH_ON;
					}

				}

			}

		}



	/* 火花呼び出し */
	if ( work->mode == GAME_SCRATCH ) {
	    if ( *work->flag ) {
		if ( ( speed = _Vu0VecLenXYZ( &work->move[foot] ) ) >= SPARK_SPEED ) {
		    _sceVu0Normalize( &move, &work->move[foot] );			
		    CallScratchSpark( &center_tmp, &normal, &move, speed, &node_mat, 1 );
		}
	    }
	    else if ( spark_flag ) {
		if ( irnd()%3 ) {
		    if ( ( speed = _Vu0VecLenXYZ( &work->move[foot] ) ) >= SPARK_SPEED ) {
			_sceVu0Normalize( &move, &work->move[foot] );			
			CallScratchSpark( &center_tmp, &normal, &move, speed, &node_mat, 0 );
		    }
		}
	    }
	}
	else {
	    if ( *work->flag ) {
		if ( ( speed = _Vu0VecLenXYZ( &work->move[foot] ) ) >= 0.0f ) {
		    _sceVu0Normalize( &move, &work->move[foot] );			
		    CallScratchSpark( &center_tmp, &normal, &move, speed, &node_mat, 1 );
		}
	    }
	}



	DG_COPY_VEC( &work->pre_pos[foot], (FVECTOR *)node_mat.m[3] );

	
	/* 接地点更新 */
	if ( scratch == SCRATCH_ON ) {

	    /* 繋ぎ特殊処理 */
	    if ( work->id[foot] == 0 ) {

			DG_COPY_VEC( &center[work->id[foot]], &center[CENTER_NUM-1] );
			DG_COPY_VEC( &width[work->id[foot]], &width[CENTER_NUM-1] );
			if ( work->pre_scratch[foot] == SCRATCH_ON ) {
				kick_flag[work->id[foot]] = 1;
			}
			else {
				kick_flag[work->id[foot]] = 0;
			}
	
			work->id[foot] = 1;

	    }
	    else if ( !(work->id[foot]%32) ) {

			DG_COPY_VEC( &center[work->id[foot]], &center[work->id[foot]-1] );
			DG_COPY_VEC( &width[work->id[foot]], &width[work->id[foot]-1] );
			if ( work->pre_scratch[foot] == SCRATCH_ON ) {
				kick_flag[work->id[foot]] = 1;
			}
			else {
				kick_flag[work->id[foot]] = 0;
			}

			work->id[foot]++;

	    }

	    DG_COPY_VEC( &center[work->id[foot]], &center_tmp );
	    DG_COPY_VEC( &width[work->id[foot]], &width_tmp );
	    if ( work->pre_scratch[foot] == SCRATCH_ON ) {
			kick_flag[work->id[foot]] = 1;
	    }
	    else {
			kick_flag[work->id[foot]] = 0;
	    }

	    if ( work->id[foot] != CENTER_NUM - 1 ) {
			kick_flag[work->id[foot]+1] = 0;		
	    }

	}




	/* 全頂点を毎フレーム更新する必要なし ->> 後で直す */
	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    for ( j = 0 ; j < N_VERTS/2 ; j++ ) {

			n = i*32+j;
	    
			if ( n == 0 ) {
		    
				_sceVu0ScaleVector( &vectmp, &width[0], SCRATCH_WIDTH );
		
				_sceVu0AddVector( pos, &center[0], &vectmp );
				pos++;
				_sceVu0SubVector( pos, &center[0], &vectmp );
				pos++;
		
			}
			else if ( j == 0 ) {
		
				DG_COPY_VEC( pos, (pos-2) );
				pos++;
				DG_COPY_VEC( pos, (pos-2) );
				pos++;
		
			}
			else {
		
				_sceVu0ScaleVector( &vectmp, &width[n], SCRATCH_WIDTH );
				
				_sceVu0AddVector( pos, &center[n], &vectmp );
				pos++;
				_sceVu0SubVector( pos, &center[n], &vectmp );
				pos++;
		
			}


		/* 後ろ半分の頂点を使ってフェードアウト */
		alpha = work->id[foot] - n;
		if ( alpha < 0 ) alpha += CENTER_NUM;		

		if ( alpha < CENTER_NUM/2 ) {
		    alpha = ALPHA;
		}
		else {
		    alpha = CENTER_NUM - alpha;
		    alpha = (int)( (float)alpha / (float)(CENTER_NUM/2) * ALPHA );
		}
		
	    
		if ( work->id[foot] == CENTER_NUM-2 ) {
		    if ( n == CENTER_NUM-1 || n == 0 || n == 1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			if ( kick_flag[n] ) {
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
			else {
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
		    }
		}
		else if ( work->id[foot] == CENTER_NUM-1 ) {
		    if ( n == 0 || n == 1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			if ( kick_flag[n] ) {
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
			else {
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
		    }
		}
		else if ( work->id[foot]%32 == 31 ) {
		    if ( n == work->id[foot]+1 || n == work->id[foot]+2 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			if ( kick_flag[n] ) {
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
			else {
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
		    }
		}
		else {		    		    
		    if ( n == work->id[foot]+1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			if ( kick_flag[n] ) {
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x0fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
			else {
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			    uvrgb->f = 0x8fff;
			    uvrgb->a = alpha;
			    uvrgb++;
			}
		    }
		}		
	    }	
	}

	/* ID更新 */
	if ( scratch == SCRATCH_ON ) {	
	    if ( ++work->id[foot] >= CENTER_NUM ) {
		work->id[foot] = 0;
	    }
	}

	/* 足接地情報確保 */
	work->pre_scratch[foot] = scratch;

    }


    // 煙呼び出し
    if ( work->mode == GAME_SCRATCH ) {

	if ( *work->flag == 1 ) {

	    _sceVu0Normalize( &work->move[0], &work->move[0] );
	    _sceVu0Normalize( &work->move[1], &work->move[1] );

	    if ( work->pre_flag == 0 ) {

		NewFatmanScratchSmoke(
				      &work->object->objs->objs[RIGHT_FOOT_NODE].world,	// 発生マトリクス
				      &work->move[0],
				      work->flag );
		NewFatmanScratchSmoke(
				      &work->object->objs->objs[LEFT_FOOT_NODE].world,	// 発生マトリクス
				      &work->move[1],
				      work->flag );
	    }

	}

    }

    work->pre_flag = *work->flag;

    work->life++;    

}


static void Die(Work *work )
{

    int i;
    
    for ( i = 0 ; i < SCRATCH_NUM ; i++ ) {
	if ( work->prim[i] ) work->prim[i] = OK_FreePrim2( work->prim[i] );
    }
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_TEX *tex )
{

    int foot;

    int		i,j,n;
    DG_PRIM2	*prim;
    FVECTOR		*pos;	
    DG_PRIM2_UVRGB	*uvrgb;


    for ( foot = 0 ; foot < SCRATCH_NUM ; foot++ ) {

	prim = work->prim[foot] = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_TEX, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
	    return 0;
	}

	prim->raise = 0;

	work->id[foot] = 0;
	work->pre_scratch[foot] = SCRATCH_OFF;
	DG_COPY_VEC( &work->move[foot], &DG_ZeroVector );

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;

	for ( i = 0 ; i < N_PRIMS; i++ ) {

	    pos   = SCR_POS;
	    uvrgb = SCR_UVS;

	    for ( j = 0 ; j < N_VERTS ; j++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;

		if ( j%2 ) {
		    uvrgb->u = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		}
		else {
		    uvrgb->u = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		}
		uvrgb->v = FTOI12( 1.0f / (float)CENTER_NUM * (float)(i*32+j/2) * tex->v_scale + tex->v_offset );

		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->b = COLOR;
		uvrgb->r = COLOR;
		uvrgb->g = COLOR;
		uvrgb->a = ALPHA;

		uvrgb++;

		if ( j % 2 ) {
		    n = ( i * N_VERTS + j ) / 2;
		    DG_COPY_VEC( &work->center[foot][n], &DG_ZeroVector );
		    DG_COPY_VEC( &work->width[foot][n], &DG_ZeroVector );
		    work->kick_flag[foot][n] = 0;
		}

	    }

	    OK_Scr_Mem( &prim->pos[ 0 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	    OK_Scr_Mem( &prim->pos[ 1 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim->uvrgb[ 0 ]))[i*N_VERTS], SCR_UVS,
					sizeof(DG_PRIM2_UVRGB), N_VERTS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim->uvrgb[ 1 ]))[i*N_VERTS], SCR_UVS,
					sizeof(DG_PRIM2_UVRGB), N_VERTS );
	    
	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

    }

    return 1;

}



static int GetResources( Work *work )
{
    
    DG_TEX	*tex;

    work->pre_flag = *work->flag;

    tex = DG_GetTexture( GV_StrCode( "smoke_lp1_alp" ) );
    if ( InitPacket( work, tex ) ) {
	return 0;
    }
    else {
	return -1;
    }

}


void *NewFatmanScratch( CONTROL *control, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->mode    = GAME_SCRATCH;
		work->control = control;
		work->object  = control->object;
		work->flag    = flag;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



/*--- デモ用呼び出し口 ---*/
void *NewDemoFatmanScratch( FMATRIX *left, FMATRIX *right, int name )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->mode = DEMO_SCRATCH;
		work->foot_node[0] = left;	/* 両足の関節取得 */
		work->foot_node[1] = right;		
		work->name = name;		/* デモ用メッセージ取得名 */
		work->demo_flag = 0;		/* デモ用フラグ初期化 */
		work->flag = &work->demo_flag;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



