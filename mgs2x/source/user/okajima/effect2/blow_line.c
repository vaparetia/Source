//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
/*
	blow_line.c
	銃弾軌跡に巻き込み

	2001/01/29 S.Okajima
	$Id: blow_line.c,v 1.1.1.3 2002/11/19 11:47:17 Yoshizawa1 Exp $

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
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define RND(_a)		( ( (BP_PS2_rand()>>16) * (_a) ) >> 15  )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_NRM		(SCRPAD_ADDR + 0x2000)
#define	SCR_WORK0	(SCRPAD_ADDR + 0x3f00)
#define	SCR_WORK1	(SCRPAD_ADDR + 0x3f10)
#define	SCR_WORK2	(SCRPAD_ADDR + 0x3f20)
#define	SCR_WORK3	(SCRPAD_ADDR + 0x3f30)

#define	N_VERTS				(64)
#define	N_VERTS_HALF		(N_VERTS/2)
#define	N_PRIMS			(1)

#if 1
#define COL_R (48)
#define COL_G (48)
#define COL_B (48)
#define COL_A (160)
#else
#define COL_R (255)
#define COL_G (255)
#define COL_B (255)
#define COL_A (255)
#endif






#define LIFE (64)

#define RADIUS_MIN (2.0f)
#define RADIUS_RND (10.0f)
#define RADIUS_ADD (100.0f)

#define CLINE_RATIO (0.5f)
#define WIDTH_WIDE_RATIO (1.01f)
#define ANGLE_DECAY_RATIO (0.97f)
#define RADIUS_ADD_DECAY_RATIO (0.95f)

#define SPEED_Z_0 (0.1f)
#define SPEED_Z_1 (0.3f)

#define SCALER_CONST (4096.0f)
#define INNER_MAX_LIMIT (SCALER_CONST * 0.80f)
#define INNER_MIN_LIMIT (SCALER_CONST * 0.25f)
#define INNER_MID_RANGE (INNER_MAX_LIMIT - INNER_MIN_LIMIT)
#define MIN_COL_RATIO (0.5f)


#define DISTANCE_MIN   (1.0f)
#define DISTANCE_MAX   (50000.0f)
#define MIN_ACCEL      (1.0f)
#define MAX_ACCEL      (200.0f)
#define ACCEL_CONSTANT (MAX_ACCEL * DISTANCE_MIN*DISTANCE_MIN)

#define STOCK_DATA_NUM (8)
int  OK_BlowLineDataCounter;
static int OK_BlowLineDataNum = 0;
static int OK_BlowLineData[STOCK_DATA_NUM];
static int OK_BlowLineDataFlag[STOCK_DATA_NUM];
static int OK_BlowLineDataPartNum[STOCK_DATA_NUM];
static FVECTOR OK_BlowLineDataVec[STOCK_DATA_NUM];

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_PRIM2	*prim;

	FMATRIX		world;

	float		angle;
	float		angle_add;
	float		max_angle;
	float		max_angle_add;
	float		radius;
	float		radius_add;

	float		length0;
	float		length0_add;
	float		length1;
	float		length1_add;
	float		length;

	FVECTOR		pos0;
	FVECTOR		pos1;

	int			life;

	int			my_id;
	int			my_status;
	float		limit_distance;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim ;
	int		clock;
	int		i;
	int		life_col;
	int		min_col;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR	*pos;
	FVECTOR	*nrm;
	FVECTOR	*fv0;
	FVECTOR	*fv1;
	FVECTOR	*fv2;
	FVECTOR	*fv3;
	float	len0;
	float	len1;
	float	step0;
	float	step1;
	float	angle;
	float	fx,fy;
	FVECTOR	fv_inner;


	if( work->my_status < 0 ){
		for( i=0; i<STOCK_DATA_NUM; i++ ){
			if( OK_BlowLineData[i] == work->my_id ){
				if( OK_BlowLineDataFlag[i] != 0 ){
					work->my_status = i;
					_sceVu0SubVector( &fv_inner, &OK_BlowLineDataVec[i], &work->pos0 ) ;
					work->limit_distance = GV_VecLen3F( &fv_inner );
				}
				break;
			}
		}
	}


	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	len0  = 0.0f;
	len1  = work->length;
	step0 = work->length0 / (float)(N_VERTS_HALF);
	step1 = work->length1 / (float)(N_VERTS_HALF);

#if 0
	alpha = COL_A * work->life / LIFE;

	pos   = SCR_POS;
	uvrgb = prim->uvrgb[ clock ];
	for( i=0; i<N_VERTS_HALF; i++ ){
		angle = work->angle - work->max_angle * (float)(i) / (float)N_VERTS_HALF;
		fx = work->radius * cosf( angle );
		fy =-work->radius * sinf( angle );

		pos->vx = fx;
		pos->vy = fy;
		pos->vz = len0;
		pos++;
		uvrgb++;

		fx *= CLINE_RATIO;
		fy *= CLINE_RATIO;

		pos->vx = fx;
		pos->vy = fy;
		pos->vz = len1;
		pos++;
		uvrgb->a = alpha;
		uvrgb++;

		len0 += step0;
		len1 += step1;
	}
	DG_SetPos( &work->world );
	pos = SCR_POS;
	DG_PutVector( pos, prim->pos[clock], N_VERTS );
#else

	fv0 = SCR_WORK0;
	fv1 = SCR_WORK1;
	fv2 = SCR_WORK2;
	fv3 = SCR_WORK3;

	DG_COPY_VEC( fv2, (FVECTOR *)DG_Chanls->eye.m[3] );

	pos   = SCR_POS;
	for( i=0; i<N_VERTS_HALF; i++ ){
		angle = work->angle - work->max_angle * (float)(i) / (float)N_VERTS_HALF;
		fx = work->radius * cosf( angle );
		fy =-work->radius * sinf( angle );

		pos->vx = fx;
		pos->vy = fy;
		pos->vz = len0;
		pos++;

		fx *= CLINE_RATIO;
		fy *= CLINE_RATIO;

		pos->vx = fx;
		pos->vy = fy;
		pos->vz = len1;
		pos++;

		len0 += step0;
		len1 += step1;
	}
	DG_SetPos( &work->world );
	pos = SCR_POS;
	DG_PutVector( pos, pos, N_VERTS );	// pos は 後で使う
	OK_Scr_Mem( prim->pos[clock], SCR_POS, sizeof(FVECTOR), N_VERTS );

	fv_inner.vx = 0.0f;
	fv_inner.vy = 0.0f;
	fv_inner.vz = 1.0f;
	DG_RotVector( &fv_inner, &fv_inner, 1 ); // 進行方向ベクトル

	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fv_inner, &fv_inner, 1 ); // カメラ方向との内積をとる
#if 0
	fv_inner.vz = (fv_inner.vz > 0.00f)? fv_inner.vz: -fv_inner.vz;
	fv_inner.vz-= 1.0f - MIN_COL_RATIO;
	fv_inner.vz = (fv_inner.vz > 0.00f)? fv_inner.vz: 0.00f;
#else
	if( fv_inner.vz > 0.0f ){
		fv_inner.vz-= (1.0f - MIN_COL_RATIO);
		fv_inner.vz = (fv_inner.vz > 0.00f)? fv_inner.vz: 0.00f;
	}else{
		fv_inner.vz+= (1.0f - MIN_COL_RATIO);
		fv_inner.vz = (fv_inner.vz < 0.00f)?-fv_inner.vz: 0.00f;
	}
#endif

	life_col = (int)( (float)(COL_A)*(1.00f - fv_inner.vz) * (float)(work->life) / (float)(LIFE) );
	min_col  = (int)( (float)(COL_A)*(        fv_inner.vz) * (float)(work->life) / (float)(LIFE) );
	// 法線生成、透視変換（回転のみ）
	nrm = SCR_NRM;
	pos = SCR_POS;
	DG_COPY_VEC( fv0, pos );
	pos++;
	DG_COPY_VEC( fv1, pos );
	pos++;
	uvrgb = prim->uvrgb[ clock ];
	uvrgb++;
	for( i=0; i<N_VERTS_HALF-1; i++ ){
		_sceVu0SubVector( fv1, fv1, fv0 );
		_sceVu0SubVector( fv0, pos, fv0 );
		_sceVu0OuterProduct( nrm, fv1, fv0 );	// 法線生成
		_sceVu0Normalize( nrm, nrm );

//printf("a:%3d::%2.4f %2.4f %2.4f \n",i,nrm->vx,nrm->vy,nrm->vz);
		_sceVu0ScaleVector( nrm, nrm, SCALER_CONST );

		_sceVu0SubVector( fv0, pos, fv2 );
		_sceVu0Normalize( fv0, fv0 );
		len0 = _sceVu0InnerProduct( nrm, fv0 );
		len0 = (len0 > 0.0f)? len0: -len0;
		if( len0 > INNER_MAX_LIMIT ){
			uvrgb->a = life_col + min_col;
		}else if( len0 < INNER_MIN_LIMIT ){
			uvrgb->a = min_col;
		}else{
			uvrgb->a = life_col * (int)(len0 - INNER_MIN_LIMIT) / (int)(INNER_MID_RANGE) + min_col;
		}
		nrm++;
		uvrgb+=2;

		DG_COPY_VEC( fv0, pos );
		pos++;
		DG_COPY_VEC( fv1, pos );
		pos++;
	}
	uvrgb->a = 0;
	uvrgb = prim->uvrgb[ clock ];
	uvrgb->a = 0;

#endif

	if ( GV_PauseLevel == 0 ){
		work->angle     -= work->angle_add;
		work->max_angle += work->max_angle_add;
		work->radius    += work->radius_add;
		work->angle_add *= ANGLE_DECAY_RATIO;

		work->length0   += work->length0_add;
		work->length1   += work->length1_add;

		if( work->my_status==-1 ){
			work->radius_add*=RADIUS_ADD_DECAY_RATIO;
		}else{
			if( work->limit_distance - work->length < 0.0f ){
				work->length *= 0.95f;
			}else{
				work->radius_add*=RADIUS_ADD_DECAY_RATIO;
			}
			if( work->length0 > work->limit_distance ) work->length0 = work->limit_distance;
			if( work->length1 > work->limit_distance ) work->length1 = work->limit_distance;
		}


		if( work->life-- <= 0 ) GV_DestroyActor( work ) ;
//		GV_DestroyActor( work ) ;
	}



}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int	i;

	work->prim = OK_FreePrim2( work->prim );

	for( i=0; i<STOCK_DATA_NUM; i++ ){
		if( OK_BlowLineData[i] == work->my_id ){
//printf("des:::%d:%d:%d::%d\n",i,OK_BlowLineDataNum,work->my_id,OK_BlowLineDataPartNum[i]);
			OK_BlowLineDataPartNum[i]--; // パート内登録数減少
			if( OK_BlowLineDataPartNum[i] <= 0 ){
				OK_BlowLineDataPartNum[i] = 0;
				OK_BlowLineDataNum--; // 登録数減少
				if( OK_BlowLineDataNum < 0 ){
					OK_BlowLineDataNum = 0;
				}
				OK_BlowLineData[i] = -1; // 初期化、新規登録許可
//printf("des:e:%d:%d:%d::%d\n",i,OK_BlowLineDataNum,work->my_id,OK_BlowLineDataPartNum[i]);
			}
			break;
		}
	}

}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i, k ;
	int		u0,u1;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );

	pos   = SCR_POS;
	uvrgb = SCR_NRM;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos++, &DG_ZeroVector );

#if 0
			uvrgb->u = (k&1)?u1:u0;
			uvrgb->v = FTOI12((float)(k/2) * 2.0f/(float)(N_VERTS-2) * tex->v_scale + tex->v_offset );
#else
			uvrgb->u = FTOI12((float)(k/2) * 2.0f/(float)(N_VERTS-2) * tex->u_scale + tex->u_offset );
			uvrgb->v = (k&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
#endif
//printf("%f\n",(float)(k/2) * 2.0f/(float)(N_VERTS-2));
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->f = (k<2)? 0x8fff: 0x0fff;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = (k&1)? COL_A: 0;
			uvrgb++;
		}
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_NRM, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_NRM, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

}

static int GetResources( Work *work, int counter, FVECTOR *pos, FVECTOR *before_pos, int flag )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR		rot;
	FVECTOR		fvtemp;
	int	i;

	work->limit_distance = FLOAT_MAX;

	work->life = LIFE;

//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
	tex = DG_GetTexture( 10646765 /*"sonic02_alp"*/ );


	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->angle     = TPI*rnd();
	work->max_angle = TPI*(0.5f + rnd()*0.5f);
	work->radius    =  RADIUS_MIN + RADIUS_RND*rnd();

//	work->angle_add     = TPI         * (0.25f + rnd()*0.75f) / (float)work->life;
	work->angle_add     = TPI*2.0f    * (0.25f + rnd()*0.75f) / (float)work->life;
	work->max_angle_add =  PI         * (0.25f + rnd()*0.75f) / (float)work->life;
	work->radius_add    =  RADIUS_ADD * (0.25f + rnd()*0.75f);


	if( (irnd()>>8)&1 ){
		work->angle_add     =-work->angle_add;
		work->max_angle     =-work->max_angle;
		work->max_angle_add =-work->max_angle_add;
	}


	DG_COPY_VEC( &work->pos0, before_pos );
	DG_COPY_VEC( &work->pos1, pos );
	_sceVu0SubVector( &fvtemp, &work->pos1, &work->pos0 ) ;

	work->length      = GV_VecLen3F( &fvtemp );
	work->length0     = 0.0f;
	work->length0_add = work->length * SPEED_Z_0;
	work->length1     = 0.0f;
	work->length1_add = work->length * SPEED_Z_1;

	OK_DirVecXY( &work->pos0, &work->pos1, &rot );
	DG_SetPos2( &work->pos0, &rot );
	DG_GetPos( &work->world );


	// 親探し
	work->my_id = counter;
//	if( OK_BlowLineDataNum >= STOCK_DATA_NUM - 1 ){
//		work->my_id = (void *)(-1); // 新規登録不能な為、このＡＣＴはチェックを行なわない
//	}else{
//		work->my_id = id;
//	}
	if( OK_BlowLineDataNum==0 ){ // 初期化
//printf("-----------------------a\n");
		for( i=0; i<STOCK_DATA_NUM; i++ ){
			OK_BlowLineData[i] = -1;
		}
	}

	for( i=0; i<STOCK_DATA_NUM; i++ ){
//printf("-----------------------b\n");
		if( OK_BlowLineData[i] == work->my_id ){ // 既に兄弟が存在
//printf("-----------------------b:0\n");
			OK_BlowLineDataFlag[i] = flag; // 状況を報告
			OK_BlowLineDataPartNum[i]++;	// 登録数増加
			break;
		}
	}
	if( i==STOCK_DATA_NUM ){ // 新規登録
//printf("-----------------------c\n");
		for( i=0; i<STOCK_DATA_NUM; i++ ){
			if( OK_BlowLineData[i] == -1 ){
				OK_BlowLineDataNum++;
				OK_BlowLineData[i] = work->my_id;
				OK_BlowLineDataFlag[i] = flag; // 状況を報告
				OK_BlowLineDataPartNum[i]++;	// 登録数増加
				DG_COPY_VEC( &OK_BlowLineDataVec[i], pos );
//printf("%d:%d:%d::%d\n",i,OK_BlowLineDataNum,work->my_id,OK_BlowLineDataPartNum[i]);
				break;
			}
		}
	}

	work->my_status = -1;

	return 0 ;
}

/* ---------------------------------------------------------------- */
void *NewBlowLine( int counter, FVECTOR *pos, FVECTOR *before_pos, int flag )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		if ( GetResources( work, counter, pos, before_pos, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

