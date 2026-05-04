//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    shower.c
    放尿エフェクト
    2001/04/25 Yuuta Kunibe	
    $Id: shower.c,v 1.1.1.3 2002/11/19 11:44:48 Yoshizawa1 Exp $
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


#define COL_R	(128)
#define COL_G	(128)
#define COL_B	(128)
#define ALPHA	(4)

#define SIZE	(30.0f)

#define N_PRIMS	(2048)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#if 0
#define INIT_PX	(13708.0f)
#define INIT_PY	( 3500.0f)
#define INIT_PZ	(-4400.0f)
#else
#define INIT_PX	( 2000.0f)
#define INIT_PY	( 2000.0f)
#define INIT_PZ	(-157000.0f)
#endif

#define	N_NEW	(15)

#define INIT_VZ (20.0f)
#define GRAVITY	(1.0f)

#define HOLD	(0x00)
#define SPLASH	(0x01)
#define HOMING	(0x02)
#define	FINISH	(0x04)


extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count ) ;



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;
    DG_PRIM2	*prim_light;

    OBJECT	*body;

    FVECTOR	vec[N_PRIMS];
    int 	id;

    float	init_speed;
    SVECTOR	rot;
    FVECTOR	vec_pre;
    int 	alpha_flg;

    int		*flag;
    int 	step;

    int 	pre_id;

    int		cnt;
    int 	off_flag;
    int 	homing_flag;
    float	param;

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int			i,n;
    int	  		clock;
    int 		flg;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    FMATRIX		mat;
    FVECTOR		fvtmp;
    float		random;

    FVECTOR  		*pos_light;
    DG_PRIM2_UVRGBWH 	*uvrgb_light;
    int 		light_id;

    int			alpha_cnt;
    int 		cnt;
    int			hit_flag;


    SVECTOR   rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;
    

    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    DG_SwitchBuffPrim2( work->prim_light );
    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];
    vec 	= work->vec;
    pos_light   = work->prim_light->pos[work->prim_light->buffer_clock];
    uvrgb_light = work->prim_light->uvrgb[work->prim_light->buffer_clock];



    flg = 0;
    alpha_cnt = 0;
    light_id = irnd()%N_PRIMS;
    hit_flag = 0;





    //if ( *work->flag == 0 ) {

    if ( *work->flag == 1 ) {
	work->init_speed = 30.f;
	work->off_flag = 0;
	work->step = 0;
    }


    /*if ( GV_PadData[1].press & PAD_X ) {
	if ( work->homing_flag == 0 ) work->homing_flag = 1;
	else work->homing_flag = 0;
    }*/

    
    if ( work->off_flag == 0  ) {

	DG_VisiblePrim2( work->prim );
	DG_VisiblePrim2( work->prim_light );

    /* 一定距離でホーミング */
	/*len2 = ( work->body->objs->objs[0].world.m[3][0] - GM_PlayerPosition.vx )
	* ( work->body->objs->objs[0].world.m[3][0] - GM_PlayerPosition.vx )
	+ ( work->body->objs->objs[0].world.m[3][2] - GM_PlayerPosition.vz )
	* ( work->body->objs->objs[0].world.m[3][2] - GM_PlayerPosition.vz );
    
    if ( len2 <= 2500.f * 2500.f && *work->flag == 0 ) {
	work->homing_flag = 1;
	if ( work->param < 0.05f ) {
	    work->param += 0.002f;
	}
    }
    else {
	work->homing_flag = 0;
	if ( work->param > 0.0f ) {
	    work->param -= 0.001f;
	}
    }*/
    
    
	/* 通常処理 */
	/*----------------*/
	/* スプライト更新 */
	/*----------------*/
	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    if ( uvrgbwh_pre->a > 0 ) {

		/* 速度更新 */
		vec->vx *= 0.99f;
		vec->vy -= GRAVITY;
		vec->vz *= 0.99f;


		/* テスト誘導 */
		if ( work->param > 0.0f ) {
		    n = work->id - i;
		    if ( n < 0 ) n += N_PRIMS;	
		    if ( n > 750 && pos->vy > GM_PlayerPosition.vy ) {
			/* ノーマルと主観でターゲット位置変える */
			if ( GM_CheckPlayerStatus( PLAYER_WATCH ) ) {
			    _sceVu0SubVector( &fvtmp, (FVECTOR *)(&DG_Chanls[0].eye.m[3]), pos );
			}
			else {
			    _sceVu0SubVector( &fvtmp, (FVECTOR *)GM_PlayerBody->objs->objs[12].world.m[3], pos );
			}
			_sceVu0Normalize( &fvtmp, &fvtmp );
			fvtmp.vx = fvtmp.vx * vec->vy / fvtmp.vy;
			fvtmp.vz = fvtmp.vz * vec->vy / fvtmp.vy;
			fvtmp.vy = vec->vy;
			/*vec->vx = vec->vx * 0.95f + fvtmp.vx * 0.05f;
			  vec->vz = vec->vz * 0.95f + fvtmp.vz * 0.05f;*/
			vec->vx = vec->vx * ( 1.0f - work->param ) + fvtmp.vx * work->param;
			vec->vz = vec->vz * ( 1.0f - work->param ) + fvtmp.vz * work->param;
		    
		    }
		    
		}
	    
	    
	    
		/* スプライト位置更新 */
		_sceVu0AddVector( pos, pos_pre, vec );
	    
	    
		if ( pos->vy < 0.0f ) {
		    uvrgbwh->a = 0;


		    /*仮跳ね返り*/
		    if ( hit_flag == 0 ) {
			DG_COPY_MAT( &mat, &DG_UnitMatrix );
			DG_COPY_VEC( (FVECTOR *)mat.m[3], pos );
			mat.m[3][1] = 0.0f;
			mat.m[3][3] = 1.0f;
			rot.vx = 3072 - 512;
			rot.vy = 4096 - 512;
			rot.vz = 0;
			rot_wide.vx = 1024;
			rot_wide.vy = 1024;
			rot_wide.vz = 1 ;
			color.vx = 32.0f;
			color.vy = 32.0f;
			color.vz = 32.0f;
			color.vw = 50.0f;
			NewSpark2( 12,		/* 発生火花数 */
				   &mat,		/* マトリクス */
				   1.0F,		/* 最小スピード */
				   80.0F,		/* 幅スピード */
				   0.0F,		/* 重力 */
				   &rot, &rot_wide,	/* 回転 回転幅 */
				   &color,		/* 色 */
				   1.0F,		/* スピードに対する火の長さの割合 */
				   3 );		/* 生存フレーム数 */

			hit_flag = 1;
		    }
		
		}
		else {
		    uvrgbwh->a = uvrgbwh_pre->a;
		}
	    

		/* ちょっとだけサイズ大きく */
		if ( !( work->cnt % 7 ) ) {
		    uvrgbwh->w = uvrgbwh_pre->w + 1;
		    uvrgbwh->h = uvrgbwh_pre->h + 1;
		}
		else {
		    uvrgbwh->w = uvrgbwh_pre->w;
		    uvrgbwh->h = uvrgbwh_pre->h;
		}
	    
	    }
	    else {

		uvrgbwh->w = 0;
		uvrgbwh->h = 0;
		uvrgbwh->a = 0;
		alpha_cnt++;
	    
	    }


	    /* キラキラ表示 */
	    if ( i == light_id ) {
		if ( uvrgbwh->a > 0 ) {
		    DG_COPY_VEC( pos_light, pos );
		    uvrgb_light->a = 128;
		}
		else {
		    uvrgb_light->a = 0;
		}
	    }	    

	    
	    /* ポインタ更新 */
	    pos++;
	    pos_pre++;
	    uvrgbwh++;
	    uvrgbwh_pre++;
	    vec++;

	}


    /*------------------------------------------*/
    /* idで指定されたスプライトを初期化し再利用 */
    /*------------------------------------------*/
    pos = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];

    /* 初期位置代入 */
    /*object = GM_PlayerBody;
    mat = object->objs->objs[0].world;    
    DG_COPY_VEC( &pos[work->id], mat.m[3] );*/

    /* 初速度計算 */
    /*DG_COPY_VEC( &work->vec[work->id], &DG_ZeroVector );

    random = ( 1.0f * frnd() - 0.5f );
    work->vec[work->id].vx += mat.m[0][0] * random;
    work->vec[work->id].vy += mat.m[0][1] * random;
    work->vec[work->id].vz += mat.m[0][2] * random;

    random = ( 1.0f * frnd() - 0.5f );
    work->vec[work->id].vx += mat.m[1][0] * random;
    work->vec[work->id].vy += mat.m[1][1] * random;
    work->vec[work->id].vz += mat.m[1][2] * random;

    work->vec[work->id].vx += mat.m[2][0] * 12.0f;
    work->vec[work->id].vy += mat.m[2][1] * 12.0f;
    work->vec[work->id].vz += mat.m[2][2] * 12.0f;*/


    /* 当たりテスト用 */
    /*DG_COPY_VEC( &pos[work->id], &DG_ZeroVector );
    pos[work->id].vx = INIT_PX;
    pos[work->id].vy = INIT_PY;
    pos[work->id].vz = INIT_PZ;

    DG_COPY_VEC( &work->vec[work->id], &DG_ZeroVector );
    work->vec[work->id].vx = ( 2.0f * frnd() - 1.0f );
    work->vec[work->id].vy = ( 2.0f * frnd() - 1.0f );
    work->vec[work->id].vz = INIT_VZ;*/

    /*-------------*/
    /* 補間version */
    /*-------------*/
    /* 現在プレイヤー位置から誘導初速度を算出 */
    /*dx = work->center.vx - INIT_PX;
    dz = work->center.vz - INIT_PZ;
    work->init_speed = bp_sqrtf( GRAVITY * ( dx * dx + dz * dz ) / INIT_PY );*/  //BP_MATH - emulate PS2 sqrtf
    /*dx = INIT_PX - GM_PlayerBody->objs->objs[12].world.m[3][0];
    dy = INIT_PY - GM_PlayerBody->objs->objs[12].world.m[3][1];
    dz = INIT_PZ - GM_PlayerBody->objs->objs[12].world.m[3][2];
    work->init_speed = bp_sqrtf( GRAVITY * ( dx * dx + dz * dz ) / dy );*/ //BP_MATH - emulate PS2 sqrtf

    /* rot_y */
    /* 角度更新 */
    if ( work->rot.vy > 120 && work->rot.vy < 2048 ) {
	work->rot.vy -= BP_PS2_rand()%34;
    }
    else if ( work->rot.vy < 3876 && work->rot.vy > 2048 ) {
	work->rot.vy -= BP_PS2_rand()%34;
    }
    else {
	work->rot.vy = BP_PS2_rand()%34 - 17;
    }
    /* 更新した角度補正 */
    if ( work->rot.vy > 4096 ) {
	work->rot.vy -= 4096;
    }
    else if ( work->rot.vy < 0 ) {
	work->rot.vy += 4096;
    }

    /* rot_x */
    /* 角度更新 */
    if ( work->rot.vx > 240 && work->rot.vx < 2048 ) {
	work->rot.vx -= BP_PS2_rand()%68;
    }
    else if ( work->rot.vx < 3756 && work->rot.vx > 2048 ) {
	work->rot.vx -= BP_PS2_rand()%68;
    }
    else {
	work->rot.vx = BP_PS2_rand()%68 - 34;
    }
    /* 更新した角度補正 */
    if ( work->rot.vx > 4096 ) {
	work->rot.vx -= 4096;
    }
    else if ( work->rot.vx < 0 ) {
	work->rot.vx += 4096;
    }

    /* 発生元マトリクス作成 */
    /*DG_SetPos( &DG_UnitMatrix );
    DG_SetPos2( &DG_ZeroVector, &work->rot );
    DG_GetPos( &mat );*/

    /*DG_SetPos( &work->body->objs->objs[0].world );
    DG_GetPos( &mat );*/
    DG_COPY_MAT( &mat, &work->body->objs->objs[0].world );
    
    cnt = 0;
    
    /* パーティクル初期化 */
    for ( i = 0 ; i < N_NEW ; i++ ) {

	
	/* 初期位置 */
	DG_COPY_VEC( &pos[work->id], &DG_ZeroVector );	
	/*pos[work->id].vx += frnd()*10.f;
	pos[work->id].vy += frnd()*10.f;
	pos[work->id].vz += frnd()*10.f;*/
	
	/*pos[work->id].vx = INIT_PX;
	pos[work->id].vy = INIT_PY - 4.f * (float)i * (float)i / 800.f;
	pos[work->id].vz = INIT_PZ;// + 2.f * (float)i;*/


	/*DG_COPY_VEC( &pos[work->id], (FVECTOR *)(&work->body->objs->objs[0].world.m[3]) );
	_sceVu0ScaleVector( &fvtmp, (FVECTOR *)(&work->body->objs->objs[0].world.m[1]),
			    (-4.f * (float)i * (float)i / 800.f ) );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );

	//pos[work->id].vy -= 4.f * (float)i * (float)i / 800.f;
	_sceVu0ScaleVector( &fvtmp, (FVECTOR *)(&work->body->objs->objs[0].world.m[2]),
			    ( 2.f * (float)i ) );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	
	_sceVu0ScaleVector( &fvtmp, &work->vec_pre, (float)i * 0.10f );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );*/


	pos[work->id].vx = frnd()*5.f;
	pos[work->id].vy = frnd()*5.f;
	pos[work->id].vz = frnd()*5.f;

	DG_COPY_VEC( &fvtmp, (FVECTOR *)(work->body->objs->objs[0].world.m[3]) );
	fvtmp.vy -= 120.f;
	_sceVu0ScaleVector( &fvtmp, &fvtmp, (float)( N_NEW - i )/(float)N_NEW );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	_sceVu0ScaleVector( &fvtmp, &pos[work->pre_id], (float)( i )/ (float)N_NEW );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	

	/* 初速度 */
	DG_COPY_VEC( &work->vec[work->id], &DG_ZeroVector );

	if ( !( irnd()%3 ) ) {
	    random = ( 1.0f * frnd() - 0.5f ) * 0.5f;/**/
	}
	else {
	    random = 0.f;
	}
	_sceVu0ScaleVector( &fvtmp, (FVECTOR *)(mat.m[0]), random );
	_sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );
	
	if ( !( irnd()%3 ) ) {
	    random = ( 1.0f * frnd() - 0.5f ) - 1.0f * (float)i / (float)N_NEW;
	}
	else {
	    random = - 1.0f * (float)i / (float)N_NEW;
	}
	DG_COPY_VEC( &fvtmp, &DG_ZeroVector );
	fvtmp.vy = 1.0f;
	_sceVu0ScaleVector( &fvtmp, &fvtmp, random );
	_sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );

	random = (float)i / (float)N_NEW;
	//_sceVu0ScaleVector( &fvtmp, (FVECTOR*)(&mat.m[2]), INIT_VZ * (1.0f - random) );
	_sceVu0ScaleVector( &fvtmp, (FVECTOR*)(&mat.m[2]), work->init_speed * ( 1.0f - random ) );
	_sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );
	//_sceVu0ScaleVector( &fvtmp, &work->vec_pre, INIT_VZ * random );
	_sceVu0ScaleVector( &fvtmp, &work->vec_pre, work->init_speed * random );
	_sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );

	if ( work->step == 0 ) {
	    /*if ( *work->flag != 0 ) {*/
	    if ( *work->flag == 0 ) {
		if ( work->init_speed > 4.f ) {
		    work->init_speed *= 0.9994f;
		}
		else {
		    work->init_speed = INIT_VZ*0.6f;
		    work->step = 1;
		}
	    }
	    else {
		if ( work->init_speed > INIT_VZ ) {
		    work->init_speed *= 0.9995f;
		}
	    }
	}
	else if ( work->step == 1 ) {
	    if ( work->init_speed > 4.f ) {
		work->init_speed *= 0.999f;
	    }
	    else {
		work->init_speed = 0.0f;
		work->step = 2;
	    }
	}
	else {
	    work->init_speed = 0.0f;
	}
	
	/* アルファ初期化 */	
	if ( work->init_speed == 0.0f ) {
	    uvrgbwh[work->id].a = 0;
	}
	else if ( work->init_speed < 13.f ) {
	    if ( !( irnd()%( (int)( 15.f - work->init_speed ) ) ) ) {
		uvrgbwh[work->id].a = ALPHA;
		cnt++;
	    }
	    else {
		uvrgbwh[work->id].a = 0;
	    }	    		
	}
	else {
	    uvrgbwh[work->id].a = ALPHA;
	    cnt++;
	}	    
	uvrgbwh[work->id].w  = SIZE;
	uvrgbwh[work->id].h  = SIZE;


	/* ID更新 */
	work->id++;

    }

#if 0    
    if ( *work->flag == 0 ) printf("%f %d\n",work->init_speed,cnt);
#endif    

    if ( work->id >= N_PRIMS ) work->id = 0;

    /* 速度ベクトル確保 */
    work->pre_id = work->id - N_NEW;
    if ( work->pre_id < 0 ) work->pre_id += N_PRIMS;
    DG_COPY_VEC( &work->vec_pre, (FVECTOR *)(mat.m[2]) );

    if ( work->step == 2 && alpha_cnt >= N_PRIMS ) work->off_flag = 1;    

    }
    else {
	DG_InvisiblePrim2( work->prim );
	DG_InvisiblePrim2( work->prim_light );
    }


    work->cnt++;
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
    work->prim_light = OK_FreePrim2( work->prim_light );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	/* 本体パーティクル初期化 */
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = prim->pos[0];
	uvrgbwh   = prim->uvrgb[0];
	vec	  = work->vec;

	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = ALPHA;
	    
	    uvrgbwh->w  = SIZE;
	    uvrgbwh->h  = SIZE;

	    DG_COPY_VEC( &work->vec[i], &DG_ZeroVector);

	    pos++;
	    uvrgbwh++;
	    vec++;
	}

	OK_Scr_Mem( prim->pos[ 1 ],   prim->pos[ 0 ], sizeof(FVECTOR),            N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], prim->uvrgb[ 0 ], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );


	/* キラキラ初期化 */
	work->prim_light->raise = 0;

	tex = DG_GetTexture( GV_StrCode( "light05_msk" ) );

	DG_ConfigPrim2Tex( work->prim_light, tex );
	DG_SetPrim2Alpha( work->prim_light, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	
	uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = COL_R;
	uvrgbwh->g  = COL_G;
	uvrgbwh->b  = COL_B;
	uvrgbwh->a  = 128;
	
	uvrgbwh->w  = 30.f;
	uvrgbwh->h  = 30.f;

	OK_Scr_Mem( work->prim_light->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          1 );
	OK_Scr_Mem( work->prim_light->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          1 );
	OK_Scr_Mem( work->prim_light->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 1 );
	OK_Scr_Mem( work->prim_light->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 1 );

	work->prim_light->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, OBJECT *body, FVECTOR *target_pos, int *flag )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->body = body;
    work->flag = flag;
    
    work->id         = 0;
    work->pre_id     = 0;
    work->init_speed = 0.0f;
    work->step       = 0;
    work->cnt 	     = 0;
    work->off_flag   = 1;
    work->homing_flag = 0;

    work->rot.vx = 0;
    work->rot.vz = 0;
    work->rot.vy = 0;
    DG_COPY_VEC( &work->vec_pre, &DG_ZeroVector );

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS/32, 32 );
    work->prim_light = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );

    tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
    InitPacket( work, prim, tex );    

    return 0;

}


void *NewShower( OBJECT	*body, FVECTOR *target_pos, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, body, target_pos, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


