//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_dash_fire.c
    ソリダスダッシュ炎
    2001/04/27 Yuuta Kunibe	
    $Id: solidas_dash_fire.c,v 1.1.1.3 2002/11/19 11:44:48 Yoshizawa1 Exp $
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

#include	"../../mode/demo/eft_con.h"



#define N_PRIMS		(50)
#define N_SET		(10)

#define	FIRE_TEX	( GV_StrCode( "blood_2bw_msk" ) )
#define FIRE_R		(45)
#define FIRE_G		(15)
#define FIRE_B		(12)

#define	INIT_ALPHA	(70)
#define	ALPHA_MIN	(60)

#define SIZE		(1000.0f)//(750.0f)


     
#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)
#define	RAND_FIELD_NUM	(0x2000 / 4)


#define SCR_POS_NOW 	(SCRPAD_ADDR)
#define SCR_POS_PRE 	((void *)SCR_POS_NOW + sizeof(FVECTOR) * N_PRIMS)
#define SCR_UVS_NOW 	((void *)SCR_POS_PRE + sizeof(FVECTOR) * N_PRIMS)
#define SCR_UVS_PRE 	((void *)SCR_UVS_NOW + sizeof(DG_PRIM2_UVRGBWH) * N_PRIMS)


#define WIND_MAX 	(20.f)

#define POINT_NUM 	(50)//(60)

#define FADE_TIME	(320)
//#define FADE_TIME	(160)
#define DELAY		(20)
//#define DELAY		(10)

#define	POWDER_TEX	( GV_StrCode( "bombpowder7_msk" ) )
#define	POWDER_COLOR	(128)     


/* 照り返しパラメータ */
#define	LIGHT_TEX	FIRE_TEX

#define LIGHT_R		(72)//(128)
#define LIGHT_G		(32)
#define LIGHT_B		(8)
#define LIGHT_ALPHA	(12)

#define LIGHT_SIZE	(4000.f)
#define	LIGHT_OFFSET	(-10.f)

#define SEARCH_LENGTH	(1000.f)

#define	LIFE		(900)



/*-------- 汎用火花 (足火花に使用) --------*/
extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count ) ;

/*-------- ソリダスダッシュ炎用テンプライト管理キャラ --------*/
extern int SolidusDashFireNum;
extern void *NewSolidusDashFireLight( void );

extern void SetSolDashFireLightPos( FVECTOR *sol_lit_pos, FVECTOR *rai_lit_pos,
				    float sol_lit_len2, float rai_lit_len2, float sol_lit_str, float rai_lit_str,
				    int bodyfire_flag );



static int DASH_FIRE_SE[] =  {
    SD_A_FIRERED1,
    SD_A_FIRERED2,
    SD_A_FIRERED3,
    SD_A_FIRERED4,
};

/* 炎スプライトパラメータ構造体 */
typedef struct {

    FVECTOR	vec;
    float	radius;
    float	rot;
    float	rot_add;

} FireParam;


/* 炎発生点パラメータ構造体 */
typedef struct {

    FVECTOR	point;
    FVECTOR	vec_point;
    float	vec_param;
    FMATRIX	point_mat;
    float	fade_param;

} FirePoint;


typedef	struct	{


    GV_ACT_EX	actor;

    FVECTOR	*pos;
    float	size;
    int		delay;
    int 	fade_time;
    int		cnt;

    int		*pflag;
    int		point_flag;

    int		spark_flag;
    
    /* 炎発生ポイント */
    FirePoint	point[POINT_NUM];
    int		point_id;

    /* ダッシュ炎 */
    DG_PRIM2	*prim;
    FireParam	param[N_PRIMS*N_SET];
    int  	id;

    /* 火の粉 */
    DG_PRIM2	*prim_powder;
    int		powder_id;
    FVECTOR	vec_powder[160];

    /* 地面照り返し */
    DG_PRIM2	*prim_light;

    int		sd_id;
    int		sd_count;

} Work;



/*-------- GetFloorMatrix : 床マトリクス取得関数 ----------
     HZX_FLR *floor		: 参照する床
     FVECTOR *floor_point	: 床座標
     FMATRIX *floor_mat		: 床マトリクス (出力)
---------------------------------------------------------*/     
static void GetFloorMatrix( HZX_FLR *floor, FVECTOR *floor_point, FMATRIX *floor_mat )
{
	FVECTOR	to;
	SVECTOR	floor_rot;

	/* 法線取得 */
	to.vx = floor->p1.h;
	to.vy = floor->p3.h;
	to.vz = floor->p2.h;

	/* 回転角度取得 */
	OK_DirVecXY( &to, &DG_ZeroVector, &floor_rot );

	/* 床マトリクス生成 */
	DG_SetPos2( floor_point, &floor_rot );
	DG_GetPos( floor_mat );
}



/*-------- SearchFloor : 床検索関数 ----------
       FMATRIX *mat	: 床マトリクス (出力)
       FVECTOR *pos	: 床検索参照座標 (ソリダスの足座標)
--------------------------------------------*/  
static void SearchFloor( FMATRIX *mat, FVECTOR *pos )
{

    HZX_FLR		seg;
    u_int		atr;
    int			hazard_flag;

    FVECTOR		from;
    FVECTOR		to;
    FVECTOR		floor_point;


    /* 検索点から上にシフト */
    DG_COPY_VEC( &from, pos );
    from.vy += SEARCH_LENGTH/2.f;
    DG_COPY_VEC( &to, pos );
    to.vy -= SEARCH_LENGTH/2.f;

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
	HZX_GetOnlinePoint( &floor_point );

	/* 床マトリクス生成 */
	GetFloorMatrix( &seg, &floor_point, mat );

    }
    else {			/* 床検索失敗 */
	printf("solidas_dash_fire.c L168 : fail_search_floor!\n");
	DG_COPY_MAT( mat, &DG_UnitMatrix );
	DG_COPY_VEC( (FVECTOR *)mat->m[3], pos );
    }

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

    FireParam		*param;

    FirePoint		*point;
    FirePoint		*point_pre;
    
    FVECTOR		*pos_powder;
    FVECTOR  		*pos_pre_powder;
    FVECTOR		*vec;
    
    FMATRIX		mat;
    SVECTOR   		rot;
    SVECTOR   		rot_wide;
    FVECTOR   		color;

    float		cos;
    float		ftmp;


    float		sub_x;
    float		sub_z;

    float		len2_pl;
    float		len2_sol;
    float		str_pl;
    float		str_sol;
    int			near_point_pl;    
    int			near_point_sol;
    int			bodyfire_flag;

    FVECTOR		veclight[4];

    float		*p_randam;

    int			invisible_num;

   //return;

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    DG_SwitchBuffPrim2( work->prim_light );
    DG_SwitchBuffPrim2( work->prim_powder );


    /* debug */
    /*if ( GV_PadData[1].press & PAD_L1 ) {
	work->point_flag = 1;
    }*/


    if ( work->pflag ) {
	if ( *work->pflag ) {
	    work->point_flag = 1;
	}
    }


    if ( work->point_flag == 0 ) {

	/* 新しいポイント取得 */
	point = &work->point[work->point_id];
	point_pre = &work->point[work->point_id-1];

	DG_COPY_VEC( &point->point, work->pos );

	_sceVu0SubVector( &point->vec_point, &point_pre->point, &point->point );

	_sceVu0Normalize( &point->vec_point, &point->vec_point );
	OK_DirVecXY( &DG_ZeroVector, &point->vec_point, &rot );

	point->vec_point.vx *= 2.f;
	point->vec_point.vz *= 2.f;     
	if ( work->point_id == 1 ) {
	    DG_COPY_VEC( &point_pre->vec_point, &point->vec_point );
	}
	point->vec_param  = 1.0f + frnd() * 0.5f;
	//point->fade_param = 0.995f + frnd() * 0.004f;
	point->fade_param = 0.99f + frnd() * 0.002f;
	

	/* 照り返しマトリクス生成 */
	SearchFloor( &point->point_mat, &point->point );


	/* 空中炎の場合は足火花発生しない */
	if ( work->spark_flag ) {
	
	    /* 足火花 */
	    DG_COPY_MAT( &mat, &DG_UnitMatrix );
	    DG_COPY_VEC( (FVECTOR *)mat.m[3], &point->point );
	    mat.m[3][1] -= 100.0f;		/* 出元が点にならないようにちょっと潜らせる */
	    rot.vx -= 512;
	    rot.vy -= 256;
	    rot_wide.vx = 512;
	    rot_wide.vy = 512;
	    rot_wide.vz = 1;
	    color.vx = 255.0F;
	    color.vy = 128.0F;
	    color.vz = 32.0F;
	    color.vw = 50.0F;
	    NewSpark2( 24,					/* 発生火花数 */
		       &mat,				/* マトリクス */
		       1.0F,				/* 最小スピード */
		       400.0F + 100.f * frnd(),		/* 幅スピード */
		       0.0F,				/* 重力 */
		       &rot, &rot_wide,			/* 回転 回転幅 */
		       &color,				/* 色 */
		       1.0F,				/* スピードに対する火の長さの割合 */
		       10 );				/* 生存フレーム数 */

	}

	if ( ++work->point_id >= POINT_NUM ) {
	    work->point_flag = 1;
	}

    }


    invisible_num = 0;	

    /* DELAYだけ遅れて炎走る */
    if ( work->cnt > work->delay ) {

	/* ランダムテーブル設定 */
	OK_rnd_to_scr( SCR_TMP, RAND_FIELD_NUM );

	/* 操作する頂点バッファ取得 */
	clock = work->prim->buffer_clock;

	/*pos         = work->prim->pos[clock];
	pos_pre	    = work->prim->pos[1-clock];
	uvrgbwh	    = work->prim->uvrgb[clock];
	uvrgbwh_pre = work->prim->uvrgb[1-clock];
	param       = work->param;*/

	/* スプライト更新 */
	for ( i = 0 ; i < N_SET ; i++ ) {

	    /* スクラッチパッド使ったら重くなった */
	    /*OK_Mem_Scr( SCR_POS_NOW, &work->prim->pos[clock][i*N_PRIMS],
			sizeof(FVECTOR), N_PRIMS );
	    OK_Mem_Scr( SCR_POS_PRE, &work->prim->pos[1-clock][i*N_PRIMS],
			sizeof(FVECTOR), N_PRIMS );
	    OK_Mem_Scr( SCR_UVS_NOW, &((DG_PRIM2_UVRGBWH*)(work->prim->uvrgb[clock]))[i*N_PRIMS],
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	    OK_Mem_Scr( SCR_UVS_PRE, &((DG_PRIM2_UVRGBWH*)(work->prim->uvrgb[1-clock]))[i*N_PRIMS],
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	    
	    pos		= SCR_POS_NOW;
	    pos_pre	= SCR_POS_PRE;
	    uvrgbwh	= SCR_UVS_NOW;    
	    uvrgbwh_pre = SCR_UVS_PRE;*/


	    /* 操作する頂点バッファ取得 */
	    pos         = &work->prim->pos[clock][i*N_PRIMS];
	    pos_pre	= &work->prim->pos[1-clock][i*N_PRIMS];
	    uvrgbwh	= &( (DG_PRIM2_UVRGBWH *)work->prim->uvrgb[clock] )[i*N_PRIMS];
	    uvrgbwh_pre = &( (DG_PRIM2_UVRGBWH *)work->prim->uvrgb[1-clock] )[i*N_PRIMS];
	    param       = &work->param[i*N_PRIMS];

	    /* ランダムテーブル開始位置初期化 */
	    p_randam = (float *)SCR_TMP;
	    p_randam += irnd()%20;


	    for ( j = 0 ; j < N_PRIMS*work->point_id/POINT_NUM ; j++ ) {

		/* 更新 */
		if ( uvrgbwh_pre->a > ALPHA_MIN ) {

		    _sceVu0AddVector( pos, pos_pre, &param->vec );
		    pos->vx += 12.5f * (*(p_randam++));
		    pos->vz += 12.5f * (*(p_randam++));

		    cos = vu0_Cos( param->rot );
		    uvrgbwh->w = uvrgbwh->h = param->radius * cos;

		    param->rot += param->rot_add;
		    if ( param->rot > PI ) param->rot -= TPI;
		    else if ( param->rot < -PI ) param->rot += TPI;

		    if ( uvrgbwh->a > ALPHA_MIN+5 ) {
			param->radius *= 0.99f;
			param->vec.vy += 0.5f;
			if ( clock ) {
			    uvrgbwh->a = uvrgbwh_pre->a - 1;
			}
			else {
			    uvrgbwh->a = uvrgbwh_pre->a;
			}
		    }
		    else {
			param->radius *= 0.6f;
			param->vec.vy += 0.5f;
			uvrgbwh->a = uvrgbwh_pre->a - 2;
		    }

		}
		/* 初期化 */
		else {

		    /* 発生ポイント取得 */
		    if ( work->cnt <= POINT_NUM + work->delay ) {
			n = irnd()%(work->cnt-work->delay);
		    }
		    else {
			n = irnd()%POINT_NUM;
		    }
		    
		    point = &work->point[n];

		    if ( point->vec_point.vy > 0.1f ) {

			DG_COPY_VEC( pos, &point->point );
			pos->vx += 200.f * (*(p_randam++));
			pos->vz += 200.f * (*(p_randam++));

			/* 初期パラメータ設定 */
			ftmp = 0.50f + 0.50f * (*(p_randam++));
			param->radius = work->size * ( 0.50f + ftmp );

			/*if ( work-> point_id < POINT_NUM ) {*/
			if ( work->point_flag == 0 ) {
			    param->radius *= 2.0f;
			}
			else {
			    param->radius *= point->vec_point.vy;
			}
		
			param->rot = PI / 2.0f;
			cos = vu0_Cos( param->rot );
			uvrgbwh->w = uvrgbwh->h = param->radius * cos;
			uvrgbwh->a  = INIT_ALPHA + irnd()%10;
		
			/* 初速度 */
			DG_COPY_VEC( &param->vec, &point->vec_point );
			_sceVu0ScaleVector( &param->vec, &point->vec_point, 30.f * ( 1.f - ftmp ) + 30.f );
			param->vec.vx += 25.f * (*(p_randam++));
			param->vec.vz += 25.f * (*(p_randam++));

		    }
		    else {
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			uvrgbwh->a = 0;
			invisible_num++;
		    }

		}

		pos++;
		pos_pre++;
		uvrgbwh++;
		uvrgbwh_pre++;
		param++;
	
	    }

	    /*OK_Scr_Mem( &work->prim->pos[clock][i*N_PRIMS], SCR_POS_NOW,
			sizeof(FVECTOR), N_PRIMS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGBWH*)(work->prim->uvrgb[clock]))[i*N_PRIMS], SCR_UVS_NOW,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );*/

	}






	/* 火の粉更新 */
	clock = work->prim_powder->buffer_clock;

	pos_powder     = work->prim_powder->pos[clock];
	pos_pre_powder = work->prim_powder->pos[1-clock];
	uvrgbwh        = work->prim_powder->uvrgb[clock];
	uvrgbwh_pre    = work->prim_powder->uvrgb[1-clock];
	vec            = work->vec_powder;

	/* ランダムテーブル開始位置初期化 */
	p_randam = (float *)SCR_TMP;
	p_randam += irnd()%20;

	for ( i=0 ; i<160 ; i++ ) {

	    if ( uvrgbwh_pre->a > 0 ) {

		_sceVu0AddVector( pos_powder, pos_pre_powder, vec );

		pos_powder->vx += vec->vy * 0.50f * (*(p_randam++));
		pos_powder->vz += vec->vy * 0.50f * (*(p_randam++));
		
		vec->vx *= 0.99f;
		vec->vz *= 0.99f;
	    
		_sceVu0ScaleVector( vec, vec, 1.005f );

		uvrgbwh->w = uvrgbwh_pre->w * 1.005f;
		uvrgbwh->h = uvrgbwh_pre->h * 1.005f;
		uvrgbwh->a = uvrgbwh_pre->a - 1;

	    }
	    else {
		uvrgbwh->w = 0;
		uvrgbwh->h = 0;
		uvrgbwh->a = 0;
	    }

	    pos_powder++;
	    pos_pre_powder++;
	    uvrgbwh++;
	    uvrgbwh_pre++;
	    vec++;


	}




    

	/* 操作する頂点バッファ取得 */
	clock = work->prim_light->buffer_clock;
	pos   = work->prim_light->pos[clock];


	clock = work->prim_powder->buffer_clock;

	near_point_pl  = 0;
	near_point_sol = 0;
	len2_pl  = 0.0f;
	len2_sol = 0.0f;
	str_pl  = 0.0f;
	str_sol = 0.0f;
    
	/* ランダムテーブル開始位置初期化 */
	p_randam = (float *)SCR_TMP;
	p_randam += irnd()%20;

	point = work->point;

	
	for ( i = 0 ; i < work->point_id ; i++ ) {

	    /* ソリダス,プレイヤーとの最短ポイント取得 */
	    if ( i == 0 ) {
		near_point_pl = 0;
		sub_x = point->point.vx - GM_PlayerPosition.vx;
		sub_z = point->point.vz - GM_PlayerPosition.vz;
		len2_pl = sub_x * sub_x + sub_z * sub_z;
		str_pl = point->vec_point.vy;

		near_point_sol = 0;
		sub_x = point->point.vx - work->pos->vx;
		sub_z = point->point.vz - work->pos->vz;
		len2_sol = sub_x * sub_x + sub_z * sub_z;
		str_sol = point->vec_point.vy;
	    }
	    else {
		sub_x = point->point.vx - GM_PlayerPosition.vx;
		sub_z = point->point.vz - GM_PlayerPosition.vz;
		ftmp = sub_x * sub_x + sub_z * sub_z;
		if ( ftmp < len2_pl ) {
		    near_point_pl = i;
		    len2_pl = ftmp;
		    str_pl  = point->vec_point.vy;
		}

		sub_x = point->point.vx - work->pos->vx;
		sub_z = point->point.vz - work->pos->vz;
		ftmp = sub_x * sub_x + sub_z * sub_z;
		if ( ftmp < len2_sol ) {
		    near_point_sol = i;
		    len2_sol = ftmp;
		    str_sol  = point->vec_point.vy;
		}
	    }



	    /* 炎初速度更新 */
	    if ( work->cnt > work->fade_time ) {	/* 消えていく */
		point->vec_point.vy = point->vec_point.vy * point->fade_param;
		if ( work->spark_flag ) {
		    point->point.vy = point->point.vy * 0.95f + point->point_mat.m[3][1] * 0.05f;
		}
	    }
	    else if ( work->cnt > work->delay + i && work->cnt < work->delay + 20 + i ) {
		point->vec_point.vx *= 0.95f;
		point->vec_point.vy = point->vec_point.vy * 0.8f + 2.f * 0.2f;
		point->vec_point.vz *= 0.95f;
	    }
	    else {
		point->vec_point.vx *= 0.99f;
		point->vec_point.vy = point->vec_point.vy * 0.95f + point->vec_param * 0.05f;
		point->vec_point.vz *= 0.99f;
	    }



	    /* 照り返し更新 */
	    if ( work->spark_flag ) {
		if ( point->vec_point.vy < 0.1f ) {
		    for ( j = 0 ; j < 4 ; j++ ) {
			DG_COPY_VEC( pos, &DG_ZeroVector );
			pos++;
		    }
		}
		else {

		    ftmp = LIGHT_SIZE * point->vec_point.vy / 2.0f;

		    veclight[0].vx = ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[0].vy = ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[0].vz = LIGHT_OFFSET;
		
		    veclight[1].vx =-ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[1].vy = ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[1].vz = LIGHT_OFFSET;

		    veclight[2].vx = ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[2].vy =-ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[2].vz = LIGHT_OFFSET;
		
		    veclight[3].vx =-ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[3].vy =-ftmp * ( 1.0f + 0.25f * (*(p_randam++)) );
		    veclight[3].vz = LIGHT_OFFSET;
		
		    DG_SetPos( &point->point_mat );
		    DG_PutVector( veclight, pos, 4 );
		    pos += 4;
		
		}
	    }
	    

	    /* 火の粉生成 */
	    if ( point->vec_point.vy > 0.2f ) {

		if ( !(irnd()%15) ) {
		    
		    pos_powder = &work->prim_powder->pos[clock][work->powder_id];
		    uvrgbwh    = &( (DG_PRIM2_UVRGBWH*)work->prim_powder->uvrgb[clock] )[work->powder_id];
		    vec        = &work->vec_powder[work->powder_id];

		    DG_COPY_VEC( pos_powder, &point->point );
		    pos_powder->vx += 250.f * (*(p_randam++));
		    pos_powder->vz += 250.f * (*(p_randam++));

		    ftmp = frnd()*PI;
		    uvrgbwh->w = vu0_Cos(ftmp) * ( 600.f + 200.f * (*(p_randam++)) ) * point->vec_point.vy;
		    uvrgbwh->h = vu0_Sin(ftmp) * ( 600.f + 200.f * (*(p_randam++)) ) * point->vec_point.vy;
		    uvrgbwh->a = 64 + irnd()%24;
		
		    _sceVu0ScaleVector( vec, &point->vec_point, 80.f + 40.f * (*(p_randam++)) );
		    vec->vx += vec->vy * 0.20f * (*(p_randam++));
		    vec->vz += vec->vy * 0.20f * (*(p_randam++));

		    if (++work->powder_id >= 160 ) work->powder_id = 0;

		}

	    }

	    point++;

	}


	/* ゲーム中の体燃え＆テンプライト設置 */
	/* 消えフェーズに入っていたら体燃やさない */
	if ( work->point[near_point_pl].vec_point.vy > 0.50f ) {
	    bodyfire_flag = 1;
	}
	else {
	    bodyfire_flag = 0;
	}
	/* 最短位置をテンプライト管理キャラに登録 -> solidus_dash_light.c */
	SetSolDashFireLightPos( &work->point[near_point_sol].point, &work->point[near_point_pl].point,
				len2_sol, len2_pl, str_sol, str_pl, bodyfire_flag );

	/* SE呼び出し */
	if ( work->sd_count <= 0 ) {
	    if ( work->point[near_point_pl].vec_point.vy > 0.50f ) {
		GM_SeSetMode( DASH_FIRE_SE[work->sd_id], &work->point[near_point_pl].point, GM_SEMODE_NORMAL );
	    }
	    work->sd_count = DIRECT_TICK( 6 + irnd()%10 );
	}
	else {
	    work->sd_count--;
	}
	

    }


    /*if ( ++work->cnt >= LIFE || invisible_num >= N_SET*N_PRIMS*work->point_id/POINT_NUM ) {
	printf("solidus_dash_fire destroy : life %d\n",work->cnt);
	GV_DestroyActor( work );
    }*/

#ifndef PSX2
	{
		extern int	DM_FrameSkip ;
		work->cnt += DM_FrameSkip ;
	}
#endif
    if ( ++work->cnt >= work->fade_time && invisible_num >= N_SET*N_PRIMS*work->point_id/POINT_NUM ) {
	//printf("solidus_dash_fire destroy : life %d\n",work->cnt);
	GV_DestroyActor( work );
    }
	
    
}



static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) {
	work->prim = OK_FreePrim2( work->prim );
    }
    if ( work->prim_light ) {
	work->prim_light = OK_FreePrim2( work->prim_light );
    }
    if ( work->prim_powder ) {
	work->prim_powder = OK_FreePrim2( work->prim_powder );
    }

    if ( work->pflag != NULL ) {
	SolidusDashFireNum--;
    }

}




/*-------- InitPacket : プリミティブ初期化関数 --------*/
static int InitPacket( Work *work )
{

	int			i,j;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGB		*uvrgb;

	FireParam		*param;


	/* 炎初期化 */
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_SET );
	if ( prim == NULL ) {
	    return 0;
	}
	prim->raise = 0;

	tex = DG_GetTexture( FIRE_TEX );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	
	param = work->param;

	
	for ( i = 0 ; i < N_SET ; i++ ) {

	    pos       = SCR_POS;
	    uvrgbwh   = SCR_UVS;
	
	    for ( j = 0 ; j < N_PRIMS ; j++ ){

		DG_COPY_VEC( pos, &DG_ZeroVector );

		switch ( j%4 ) {
		case 0:
		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    break;
		case 1:
		    uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    break;
		case 2:
		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    break;
		case 3:
		    uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    break;
		}

		uvrgbwh->q0 = 4096;
		uvrgbwh->f0 = 0x0fff;
		uvrgbwh->q1 = 4096;
		uvrgbwh->f1 = 0x0fff;

		uvrgbwh->r  = FIRE_R;
		uvrgbwh->g  = FIRE_G + irnd()%FIRE_G;
		uvrgbwh->b  = FIRE_B;
		uvrgbwh->a  = 0;

		param->radius = 0;
		param->rot = 0;
		param->rot_add = TPI * 0.025f * frnd();//0.075	/* 0.05f～0.10f */
		uvrgbwh->w = 0;
		uvrgbwh->h = 0;

		DG_COPY_VEC( &param->vec, &DG_ZeroVector );

		pos++;
		uvrgbwh++;
		param++;

	    }	

	    OK_Scr_Mem( &prim->pos[ 0 ][i*N_PRIMS], SCR_POS, sizeof(FVECTOR), N_PRIMS );
	    OK_Scr_Mem( &prim->pos[ 1 ][i*N_PRIMS], SCR_POS, sizeof(FVECTOR), N_PRIMS );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 0 ]) )[i*N_PRIMS], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 1 ]) )[i*N_PRIMS], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );


	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );




	/* 照り返し初期化 */
	prim = work->prim_light = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, POINT_NUM, 4 );
	if ( prim == NULL ) {
	    return 0;
	}
	prim->raise = 0;

	//tex = DG_GetTexture( LIGHT_TEX );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	for ( i = 0 ; i < POINT_NUM ; i++ ) {

	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = LIGHT_R;
	    uvrgb->g = LIGHT_G;
	    uvrgb->b = LIGHT_B;
	    uvrgb->a = LIGHT_ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = LIGHT_R;
	    uvrgb->g = LIGHT_G;
	    uvrgb->b = LIGHT_B;
	    uvrgb->a = LIGHT_ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = LIGHT_R;
	    uvrgb->g = LIGHT_G;
	    uvrgb->b = LIGHT_B;
	    uvrgb->a = LIGHT_ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = LIGHT_R;
	    uvrgb->g = LIGHT_G;
	    uvrgb->b = LIGHT_B;
	    uvrgb->a = LIGHT_ALPHA;
	    uvrgb++;
	    
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        POINT_NUM*4 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        POINT_NUM*4 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), POINT_NUM*4 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), POINT_NUM*4 );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );




	/* 火の粉初期化 */
	prim = work->prim_powder = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 10, 16 );	
	if ( prim == NULL ) {
	    return 0;
	}
	prim->raise = 0;

	tex = DG_GetTexture( POWDER_TEX );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	
	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	
	for ( i = 0 ; i < 160 ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    switch ( i%4 ) {
	    case 0:
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		break;
	    case 1:
		uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		break;
	    case 2:
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		break;
	    case 3:
		uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		break;
	    }

	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;

	    uvrgbwh->r  = POWDER_COLOR;
	    uvrgbwh->g  = POWDER_COLOR;
	    uvrgbwh->b  = POWDER_COLOR;
	    uvrgbwh->a  = 0;

	    pos++;
	    uvrgbwh++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],  SCR_POS, sizeof(FVECTOR),           160 );
	OK_Scr_Mem( prim->pos[ 1 ],  SCR_POS, sizeof(FVECTOR),           160 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 160 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 160 );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}




static int GetResources( Work *work, FVECTOR* pos )
{

    FirePoint	*point;

    
    work->cnt  = 0;

    work->pos       = pos;
    work->size      = SIZE;

    work->point_flag = 0;    
    
    point = work->point;    
    DG_COPY_VEC( &point->point, pos );
    point->vec_param  = 1.0f;
    point->fade_param = 0.99f;

    work->point_id = 1;
    work->powder_id = 0;

    work->sd_id = irnd()%4;
    work->sd_count = 0;
    
    if ( InitPacket( work ) ) {
	return 0;
    }
    else {	
	return -1;
    }

}



/*-------- NewSolidusDashFire : ソリダスダッシュ炎呼び出し関数 ----------
     FVECTOR	*pos : 炎発生位置 (ソリダスの足座標)
-----------------------------------------------------------------------*/
void *NewSolidusDashFire( FVECTOR *pos, int *pflag, int fade_time )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->pflag = pflag;

		/*--- パラメータ対応デモ用呼び出し口のためにここでパラメータ取得 ---*/
		work->delay = DELAY;
		if ( fade_time == 0 ) {
		    work->fade_time = FADE_TIME;
		}
		else {
		    work->fade_time = fade_time;
		}
		work->spark_flag = 1;
    
		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}

	}

	/*--- プレイヤー位置を見ているのでデモではダッシュ炎ライト管理キャラは呼ばない ---*/
	if ( pflag != NULL ) {
	    if ( SolidusDashFireNum == 0 ) {
		NewSolidusDashFireLight();
	    }
	    SolidusDashFireNum++;
	}

	return (void *)work ;

}

/*-------- NewDemoSolidusDashFire : デモ用ソリダスダッシュ炎呼び出し関数 ----------
     FMATRIX	*mat : 炎発生位置 (ソリダスの足先マトリクス)
-----------------------------------------------------------------------*/
void *NewDemoSolidusDashFire( FMATRIX *mat, int *pflag )
{
	return (void *)NewSolidusDashFire( (FVECTOR *)mat->m[3], pflag, FADE_TIME ) ;
}


void *NewDemoSolidusDashFire2( FMATRIX *mat, int delay, int fade_time )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->pflag = NULL;

		/*--- パラメータ対応デモ用呼び出し口のためにここでパラメータ取得 ---*/
#if defined(BP_VITA)
      work->delay = delay / 2;
#else
      work->delay = DIRECT_TICK( delay );
#endif
      
		if ( work->delay == 0 ) {
		    work->spark_flag = 0;
		}
		else {
		    work->spark_flag = 1;
		}
#if defined(BP_VITA)
      work->fade_time = fade_time / 2;
#else
		work->fade_time = DIRECT_TICK( fade_time );
#endif
    
		if ( GetResources( work, (FVECTOR *)mat->m[3] ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}

	}

	/*--- プレイヤー位置を見ているのでデモではダッシュ炎ライト管理キャラは呼ばない ---*/
	//if ( SolidusDashFireNum == 0 ) {
	//    NewSolidusDashFireLight();
	//}
	//SolidusDashFireNum++;

	return (void *)work ;

}
