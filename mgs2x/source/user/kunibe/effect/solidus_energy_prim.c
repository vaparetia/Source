//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_energy_prim.c
    ソリダス集束パーティクル
    2001/07/18 Yuuta Kunibe	
    $Id: solidus_energy_prim.c,v 1.1.1.3 2002/11/19 11:44:50 Yoshizawa1 Exp $
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




#define N_PRIMS		(32)
#define	N_VERTS		(32)
#define	N_SPRTS		(N_PRIMS*N_VERTS)


#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)
#define	SCR_NUM		(8)

#define	SWORD_POINT_NUM	(400)
#define	SNAKE_POINT_NUM	(600)
#define	MAX_POINT_NUM	(SNAKE_POINT_NUM)

#define	LIGHT_TEX	( GV_StrCode( "w_bonbori" ) )

#define	COL_R		(60)
#define	COL_G		(162)
#define	COL_B		(255)
#define ALPHA		(16)
#define	SIZE		(10)

#define	SPEED		(50.0f)

#define	MAX_DIST	(10000.0f)

#define	RAND_FIELD_NUM	(0x2000 / 4)


/* 共和刀 ＆ 民主刀 */
static	FVECTOR	KwtShift[] = {
	{ 18.0F, -135.0F, 87.0F },
	{ 18.0F, -320.0F, 800.0F },
};

static	FVECTOR	MstShift[] = {
	{ -18.0F, -135.0F, 87.0F },
	{ -18.0F, -285.0F, 607.0F },
};

static	FVECTOR	*Blade_Shift[] = {
	KwtShift, 
	MstShift,
};

/* 剣のルート */
static int Blade_Node[] = {
    HUMAN21_MIGI_TE,
    HUMAN21_HIDARI_TE,
};

/* モードフラグ */
enum {
    ENERGY_STAY         = 0x00,
    ENERGY_SWORD        = 0x01,
    ENERGY_SNAKEARM     = 0x02,
};


typedef struct {

    FMATRIX	mat;
    SVECTOR	rot;
    float	speed;
    int		count;

    int		point_id;	/* 集束点ID */

    float	param;		/* 集束パラメータ */

} PRIM_PARAM;
	

typedef	struct	{

    GV_ACT_EX	actor;
    int		name;

    OBJECT	*body;
    DG_EVMOBJ	*evmobj;

    DG_PRIM2	*prim;

    PRIM_PARAM	param[N_SPRTS];

    /* 集束点 */
    FVECTOR	gather_point[MAX_POINT_NUM];
    
    int 	*mode;
    int 	pre_mode;
    int		demo_mode;

    int		act_flag;

} Work;



int SolidusEnergyStatus = 0;
extern DG_EVMOBJ *SnakeArmEvmObj;

extern void *NewDemoBladeFlow( FMATRIX *root, FVECTOR *offset0, FVECTOR *offset1, int n_disp, int init_col, int tex_code, int *alpha );


/* 内分点算出関数 */
static inline void GetDividingPoint( FVECTOR *vec_a, FVECTOR *vec_b, float ratio_a, float ratio_b )
{
// yano /*未検証*/
#ifdef BP_PSX2_ASM
	  asm volatile ("
	  		lqc2		vf14, 0x00(%0)		# vec_a
			lqc2		vf15, 0x00(%1)		# vec_b
			qmtc2.ni	%2, vf12		# ratio_a
			qmtc2.ni	%3, vf13		# ratio_b

			vmulax.xyz	ACC,  vf14, vf12x	# acc = vec_a[0] * ratio_a
			vmaddx.xyz	vf16, vf15, vf13x	# vf16 = acc + (vec_b[0] * ratio_b)

			vaddx.w		vf16, vf00, vf00x	# vf16.vw = 1.0f

			sqc2	        vf16, 0x00(%0)
	  ": : "r"(vec_a),  "r"(vec_b), "r"(ratio_a), "r"(ratio_b) : "memory" );
#else
	FVECTOR fvtemp0, fvtemp1;
	_sceVu0ScaleVectorXYZ( &fvtemp0, vec_a, ratio_a );
	_sceVu0ScaleVectorXYZ( &fvtemp1, vec_b, ratio_b );
	_sceVu0AddVector( vec_a, &fvtemp0, &fvtemp1 );
	vec_a->vw = 1.0f;
#endif
}



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
		case 0:		/* 通常状態 */
			work->demo_mode = ENERGY_STAY;
			break;
		case 1:		/* 剣集束 */
			work->demo_mode = ENERGY_SWORD;
			break;
		case 2:		/* 蛇手集束 */
			work->demo_mode = ENERGY_SNAKEARM;
			break;				
		}

		msg++;

    }

}


static void Act( Work *work )
{

    int			i,j;

    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH	*uvrgbwh;
    DG_PRIM2_UVRGBWH	*uvrgbwh_pre;
    PRIM_PARAM			*param;

    FVECTOR		vectmp;

    float		inner_x;
    float		inner_z;


    /* 集束基準点 */
    FVECTOR		blade_pos[2][2];
    FVECTOR		sa_pos[32];

    FVECTOR		*shift;
    FVECTOR		aim_pos;

    float		diff;

    int 		mode_change;
    float		init_speed;

    int 		invisible_num;

    FVECTOR		*point;

    



	RecieveMessage( work );

    
    DG_SwitchBuffPrim2( work->prim );
    pos         = work->prim->pos[work->prim->buffer_clock];
    uvrgbwh     = work->prim->uvrgb[work->prim->buffer_clock];
    uvrgbwh_pre = work->prim->uvrgb[1-work->prim->buffer_clock];






    /* モード変更確認！*/
    init_speed = 0.0f;
    if ( *work->mode != work->pre_mode ) {

		mode_change = 1;
		work->act_flag = 1;

		if ( *work->mode == ENERGY_STAY ) {
			/* 拡散基本スピード設定 */
			if ( work->pre_mode == ENERGY_SWORD ) {
				init_speed = SPEED * 3.0f;
			}
			else {
				init_speed = SPEED * 1.5f;
			}
		}

    }
    else {
		mode_change = 0;
    }






    /* 必要な集束ノード算出 */
    if ( *work->mode == ENERGY_SWORD ) {

		point = work->gather_point;

		/* 剣のポイント算出 */
		for ( i = 0 ; i < 2 ; i++ ) {

			DG_SetPos( &work->body->objs->objs[Blade_Node[i]].world );
			shift = Blade_Shift[i];
			DG_PutVector( &Blade_Shift[i][0], &blade_pos[i][0], 1 );
			DG_PutVector( &Blade_Shift[i][1], &blade_pos[i][1], 1 );

			/* ここで集束点を作成してしまう */
			for ( j = 0 ; j < 200 ; j++ ) {
				_sceVu0ScaleVector( &vectmp, &blade_pos[i][0], 0.005f*(float)j );
				_sceVu0ScaleVector( point,   &blade_pos[i][1], 1.0f - 0.005f*(float)j );
				_sceVu0AddVector( point, point, &vectmp );
				point++;
			}

		}

    }
    else if ( *work->mode == ENERGY_SNAKEARM ) {

		point = work->gather_point;

		/* 蛇手ポイント算出 */
		for ( i = 1 ; i < 33 ; i++ ) {

			/* 関節相対位置取得 */
			vectmp.vx = work->evmobj->def->skeleton[i].rt_tx;
			vectmp.vy = work->evmobj->def->skeleton[i].rt_ty;
			vectmp.vz = work->evmobj->def->skeleton[i].rt_tz;
				
			/* 関節位置絶対化 */
			DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][i] );
			DG_PutVector( &vectmp, &sa_pos[i-1], 1 ) ;

			/* ここで集束点を作成してしまう */
			if ( i!=1 && i!=17 ) {
				for ( j = 0 ; j < 20 ; j++ ) {
               // BP - This [i-1] and [i-2] below are actually safe, because this code is only run when i
               // is >= 2, < 33, and != 17
					_sceVu0ScaleVector( &vectmp, &sa_pos[i-1], 0.05f*(float)j );
					_sceVu0ScaleVector( point,   &sa_pos[i-2], 1.0f - 0.05f*(float)j );
					_sceVu0AddVector( point, point, &vectmp );
					point++;		    
				}
			}

		}

    }



    /* 全パーティクルが消えている時は処理しない */
    if ( !work->act_flag ) {

		/* 描画スキップ */
		DG_InvisiblePrim2( work->prim );

    }
    /* 通常処理 */
    else {

		DG_VisiblePrim2( work->prim );
		invisible_num = 0;
	
		for ( i = 0 ; i < SCR_NUM ; i++ ) {

			OK_Mem_Scr( SCR_POS, &work->param[i*N_SPRTS/SCR_NUM], sizeof(PRIM_PARAM), N_SPRTS/SCR_NUM );
			param = SCR_POS;

			for ( j = 0 ; j < N_SPRTS/SCR_NUM ; j++ ) {

				/* モード変更時の初期化 */
				if ( mode_change ) {

					if ( *work->mode == ENERGY_STAY ) {
						param->speed = init_speed * ( frnd()*0.30f + 0.70f );		/**/
					}
					else {

						/* 剣バージョン */
						if ( *work->mode == ENERGY_SWORD ) {
							param->point_id = irnd()%400;
						}
						/* 蛇手バージョン */
						else if ( *work->mode == ENERGY_SNAKEARM ) {
							param->point_id = irnd()%600;
						}
			
						/* アルファ０のものは再配置 */
						if ( uvrgbwh->a == 0 ) {
							DG_COPY_VEC( (FVECTOR *)param->mat.m[3], &work->body->objs->objs[0].world.m[3] );
							param->mat.m[3][0] += frnd()*MAX_DIST;			/**/
							param->mat.m[3][1] += frnd()*MAX_DIST;			/**/
							param->mat.m[3][2] += frnd()*MAX_DIST;			/**/
							param->speed = SPEED * ( frnd()*0.30f + 0.70f );		/**/
						}

						/* 初期集束率に幅持たせる */
						param->param = 0.05f + frnd()*0.05f;				/**/

					}

				}



				if ( *work->mode == ENERGY_STAY && uvrgbwh_pre->a == 0 ) {
					/* 定常状態でアルファ０のものは処理しない */
				}
				else {

					if ( *work->mode == ENERGY_STAY ) { 
						DG_SetPos( &param->mat );
						DG_RotatePos( &param->rot );
						DG_GetPos( &param->mat );
					}

					_sceVu0ScaleVector( &vectmp, (FVECTOR *)param->mat.m[2], param->speed );
					_sceVu0AddVector( (FVECTOR *)param->mat.m[3], (FVECTOR *)param->mat.m[3], &vectmp );

					//----- 集束動作
					if ( *work->mode ) {

						//----- 集束点
						DG_COPY_VEC( &aim_pos, &work->gather_point[param->point_id] );

						if ( param->param == 1.0f ) {
							DG_COPY_VEC( (FVECTOR *)param->mat.m[3], &aim_pos );
						}
						else {

							//----- 位置補正
#if 0					
							_sceVu0ScaleVector( &vectmp, &aim_pos, param->param );
							_sceVu0ScaleVector( (FVECTOR *)param->mat.m[3],
												(FVECTOR *)param->mat.m[3], 1.0f - param->param );
							_sceVu0AddVector( (FVECTOR *)param->mat.m[3], (FVECTOR *)param->mat.m[3], &vectmp );
#else
							GetDividingPoint( (FVECTOR *)param->mat.m[3], &aim_pos, 1.0f-param->param, param->param );
#endif						    
			    
							param->param += 0.01f;
							param->speed *= 0.95f;

							/* 目標地点に到達とみなす */
							if ( fabs( aim_pos.vx - param->mat.m[3][0] ) < 100.0f
								 && fabs( aim_pos.vy - param->mat.m[3][1] ) < 100.0f
								 && fabs( aim_pos.vz - param->mat.m[3][2] ) < 100.0f ) {
				
								param->param = 1.0f;

							}

						}
			
					}
					else {
						if ( param->param != 0.0f ) {
							param->param = 0.0f;
						}
					}

					//----- 位置反映
					DG_COPY_VEC( pos, (FVECTOR *)param->mat.m[3] );

		    
					//----- 回転更新
					if ( --param->count <= 0 ) {

						_sceVu0SubVector( &vectmp, (FVECTOR *)work->body->objs->objs[0].world.m[3], (FVECTOR *)param->mat.m[3] );

						inner_x = _sceVu0InnerProduct( &vectmp, (FVECTOR *)param->mat.m[0] );
						inner_z = _sceVu0InnerProduct( &vectmp, (FVECTOR *)param->mat.m[2] );

						if ( inner_z > 0.f ) {
		    
							if ( inner_x > 0.f ) {
								param->rot.vy = 10 + irnd()%10;
							}
							else {
								param->rot.vy =-10 - irnd()%10;
							}

						}
						else {

							if ( inner_x > 0.f ) {
								param->rot.vy = 15 + irnd()%15;
							}
							else {
								param->rot.vy =-15 - irnd()%15;
							}

						}

						param->count = 15 + irnd()%15;

					}

				}



				//----- アルファアニメーション
				/* 定常状態処理 */
				if ( *work->mode == ENERGY_STAY ) {

					if ( mode_change ) {
						/* 飛び散る瞬間強烈にしてみた */
						//uvrgbwh->a = 128 - irnd()%64;
						uvrgbwh->a = 64 - irnd()%32;
					}
					else {

						/* アルファ０のパーティクルカウントしておく */
						if ( uvrgbwh->a == 0 && uvrgbwh_pre->a == 0 ) {
							invisible_num++;
						}
						else {

							/* アタック後,一定値まで急激に減衰 */
							if ( uvrgbwh_pre->a > 8 ) {
								uvrgbwh->a = uvrgbwh_pre->a * 7 / 8;
							}
							/* ゆっくり減衰 */
							else {
								if ( work->prim->buffer_clock ) {
									if ( uvrgbwh_pre->a > 0 ) {
										uvrgbwh->a = uvrgbwh_pre->a - 1;
									}
									else {
										if ( uvrgbwh->a > 0 ) {
											uvrgbwh->a = 0;
										}
									}
								}
								else {
									uvrgbwh->a = uvrgbwh_pre->a;
								}
							}
							
						}

					}

				}
				/* 集束状態処理 */
				else {
					
					/* diff算出 */
					_sceVu0SubVector( &vectmp, pos, (FVECTOR *)work->body->objs->objs[0].world.m[3] );
					diff = _sceVu0InnerProduct( &vectmp, &vectmp );

					if ( param->param == 1.0f ) {
						if ( uvrgbwh->a > 0 ) {
							uvrgbwh->a = 0;
						}
					}
					else {
		    
						if ( diff > MAX_DIST * MAX_DIST ) {
							uvrgbwh->a = 0;
						}
						else {
							uvrgbwh->a = ALPHA * ( MAX_DIST*MAX_DIST - diff ) / ( MAX_DIST*MAX_DIST );
						}

					}

				}

	        
				pos++;
				uvrgbwh++;
				uvrgbwh_pre++;
				param++;

	
			}

			OK_Scr_Mem( &work->param[i*N_SPRTS/SCR_NUM], SCR_POS, sizeof(PRIM_PARAM), N_SPRTS/SCR_NUM );

		}

#if 0	
		printf("invisible %d\n", invisible_num);
#endif	

		/* 定常状態ですべてのパーティクルのアルファが０ならば次フレームからアクト通さない */
		if ( invisible_num >= N_SPRTS ) {
			work->act_flag = 0;
		}
    
    }

    work->pre_mode = *work->mode;

    
}




static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );

}




/* スプライトバージョン */
static int InitPacket( Work *work )
{

	int			i,j;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	PRIM_PARAM		*param;

	SVECTOR			rot;

	int			random;



	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		return 0;
	}

	tex = DG_GetTexture( LIGHT_TEX );


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	param	= work->param;

	for ( i = 0 ; i < 4 ; i++ ) {

	    pos     = SCR_POS;
	    uvrgbwh = SCR_UVS;
	
	    for ( j = 0 ; j < N_PRIMS*N_VERTS/4 ; j++ ){

		DG_COPY_VEC( pos, &work->body->objs->objs[0].world.m[3] );
		pos->vx += frnd()*MAX_DIST;
		pos->vy += frnd()*MAX_DIST;
		pos->vz += frnd()*MAX_DIST;

		DG_SetPos( &DG_UnitMatrix );
		rot.vx = 0;
		rot.vy = irnd()%4096;
		rot.vz = 0;
		DG_RotatePos( &rot );
		DG_GetPos( &param->mat );
		DG_COPY_VEC( (FVECTOR *)param->mat.m[3], pos );

		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->q0 = 4096;
		uvrgbwh->f0 = 0x0fff;
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->q1 = 4096;
		uvrgbwh->f1 = 0x0fff;

		random = irnd()%10;
		uvrgbwh->w = SIZE + random;
		uvrgbwh->h = SIZE + random;

		uvrgbwh->r  = COL_R;
		uvrgbwh->g  = COL_G;
		uvrgbwh->b  = COL_B;
		uvrgbwh->a  = 0;

		param->count = 2 + irnd()%60;
		param->speed = SPEED * ( frnd()*0.30f + 0.70f );

		param->rot.vx = 0;
		param->rot.vy = 0;
		param->rot.vz = 0;

		param->param = 0.0f;
	    
		pos++;
		uvrgbwh++;
		param++;		

	    }	


	    OK_Scr_Mem( &prim->pos[ 0 ][N_PRIMS*N_VERTS/4*i], SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS/4 );
	    OK_Scr_Mem( &prim->pos[ 1 ][N_PRIMS*N_VERTS/4*i], SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS/4 );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH *)( prim->uvrgb[ 0 ] ) )[N_PRIMS*N_VERTS/4*i],
			SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS/4 );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH *)( prim->uvrgb[ 1 ] ) )[N_PRIMS*N_VERTS/4*i],
			SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS/4 );

	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}




static int GetResources( Work *work )
{

    work->pre_mode = *work->mode;

    work->act_flag = 1;
    
    if ( InitPacket( work ) ) {
		return 0;
	}
	else {
		return -1;
	}

}




void *NewSolidusEnergyPrim( OBJECT *body, int *mode )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = 0;
		work->mode = mode;		

		work->body = body;
		work->evmobj = SnakeArmEvmObj;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



void *NewDemoSolidusEnergyPrim( int name, OBJECT *body, DG_EVMOBJ *evmobj )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = name;
		work->demo_mode = ENERGY_STAY;
		work->mode = &work->demo_mode;

		work->body = body;
		work->evmobj = evmobj;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}
