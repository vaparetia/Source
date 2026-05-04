//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    rising_smoke.c
    立ち昇る煙
    2001/03/28 Yuuta Kunibe	
    $Id: demo_rising_smoke.c,v 1.1.1.3 2002/11/19 11:44:37 Yoshizawa1 Exp $
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

#define COLOR	(64)//(128)
#define ALPHA	(40)
#define SIZE	(100.0f)

#define	LIFE	(360)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define WIND_MAX (20.f)


/* グローバル風パラメータ */
extern FVECTOR	G_wind;
extern SVECTOR	G_wind_rot;
extern int	G_wind_intense;
extern int	G_wind_intense_max;
extern FMATRIX	G_wind_matrix;
extern float	OK_slow_param;

enum {
    DEMO_SMOKE_FIX,
    DEMO_SMOKE_LINK,
    DEMO_SMOKE_DUMMY,
};

typedef struct {

    FVECTOR	vec;
    float	radius;
    float	cos;
    float	sin;

} SMOKE_PARAM;


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    int 	num;
    int		mode;
    
    FMATRIX	*mat;
    FMATRIX	matrix;
    EFTCONTROL  *eft_ctrl;

    float	size;
    float	init_size;
    float	last_size;
    float	rising_speed;
    int		alpha;

    float	init_speed;
    FVECTOR	wind;

    /* 発生ベクトル振り用 */
    SVECTOR	rnd_rot;
    int		rot_x_flag;
    int		rot_y_flag;

    int  	id;
    int 	life;
    int		cnt;
    int		loop_flag;

    SMOKE_PARAM	param[0];    

} Work;



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
    float		intense;
    float		len;
    float		ftmp;
    float		angle;
    FVECTOR		vectmp;
    FVECTOR		prevpos;
    FVECTOR		unit_vec;
    FVECTOR		local_wind;
    int			alpha;
    FMATRIX		mattmp;

    float		rate;
    
    SMOKE_PARAM		*param;

    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    if ( work->mode == DEMO_SMOKE_DUMMY ) {
       DM_EftControlMatrix( work->eft_ctrl, &work->matrix );
    }

    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];

    param = work->param;

    /* 風計算 */
	{/*0DIV 2002.03.14 yano*/
		float dmdm = (float)G_wind_intense_max;
		if( dmdm == 0.0f ) dmdm = 0.0000001f;
		intense = WIND_MAX / dmdm;
	}
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;


    n = work->id - 1;
    if ( n < 0 ) n = work->num - 1;

    /* スローパラメータ対応デバッグ */
    /*if ( GV_PadData[1].press & PAD_L2 ) {
	if ( OK_slow_param == 1.0f) {
	    OK_slow_param = 0.0f;
	}
	else {
	    OK_slow_param = 1.0f;
	}
    }*/



    /* スプライト更新 */
    for ( i = 0 ; i < work->num ; i++ ) {	

	
	ftmp = 1.0f + frnd() * 0.2f;
	_sceVu0ScaleVector( &vectmp, &work->wind, ftmp );
	_sceVu0AddVector( pos, pos_pre, &vectmp );

	_sceVu0ScaleVector( &vectmp, &param->vec, OK_slow_param );
	_sceVu0AddVector( pos, pos, &vectmp );
	param->vec.vy += work->rising_speed * OK_slow_param;

	if ( OK_slow_param == 1.0f ) {
	    param->vec.vx *= 0.98f;
	    param->vec.vz *= 0.98f;
	}
	else if ( OK_slow_param > 0.0f ) {
	    rate = powf( 0.98f, OK_slow_param );
	    param->vec.vx *= rate;
	    param->vec.vz *= rate;
	}

	j = work->id-1 - i;
	if ( j < 0 ) j += work->num;

	if ( work->loop_flag ) {
	    alpha = work->alpha * ( work->num - j ) / work->num;	
	}
	else {
	    if ( work->id == 0 ) {
		alpha = 0;
	    }
	    else {
		if ( i > work->id ) {
		    alpha = 0;
		}
		else {
		    alpha = work->alpha * i / work->id;
		}
	    }
	}

	uvrgbwh->a = alpha;
	if ( uvrgbwh->a > 0 ) {
	    ftmp = param->radius * (float)( work->num - j ) / (float)work->num
		+ work->last_size * (float)j / (float)work->num;
	    uvrgbwh->w = ftmp * param->cos;
	    uvrgbwh->h = ftmp * param->sin;
	}
	else {
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	}
		
	if ( i == n ) DG_COPY_VEC( &prevpos, pos );

	pos++;
	pos_pre++;
	uvrgbwh++;

	param++;

    }



    /* 前の煙との直線距離から発生させる煙の数を算出 */
    _sceVu0SubVector( &vectmp, &prevpos, (FVECTOR *)work->mat->m[3] );
    len = bp_sqrtf( _sceVu0InnerProduct( &vectmp, &vectmp ) ); //BP_MATH - emulate PS2 sqrtf

    n = (int)( len / ( work->size / 2.f ) );

    if ( OK_slow_param == 1.0f ) {
	n += 1;
    }
    else if ( OK_slow_param == 0.0f ) {
	n = 0;
    }
    else {
	if ( !( work->cnt % ( (int)(1.0f/OK_slow_param) ) ) ) {
	    n += 1;
	}
    }
    
    if ( n > 0 ) {

	ftmp = 1.0f / (float)n;
	_sceVu0SubVector( &unit_vec, (FVECTOR *)work->mat->m[3], &prevpos );
	_sceVu0ScaleVector( &unit_vec, &unit_vec, ftmp );
   
	work->init_size = work->init_size * ( 1.f + frnd()*0.1f );

	if (!(irnd()%120)) work->init_size = work->size * 1.25;
    
	if ( work->init_size > work->size * 1.25f ) {
	    work->init_size = work->size * 1.25f;
	}
	else if ( work->init_size < work->size * 0.8f ) {
	    work->init_size = work->size * 0.8f;
	}



	// 発生方向を一定角度内でランダムに振る
	// Ｘ軸回転
	switch ( work->rot_x_flag ) {
    case 0:
	work->rnd_rot.vx += irnd()%32-16;
	break;
    case 1:
	work->rnd_rot.vx += irnd()%32-24;
	if ( work->rnd_rot.vx <= 64 ) {
	    work->rot_x_flag = 0;
	}
	break;
    case 2:
	work->rnd_rot.vx += irnd()%32-8;
	if ( work->rnd_rot.vx >=-64 ) {
	    work->rot_x_flag = 0;
	}
	break;
    }

    if ( work->rnd_rot.vx > 256 ) {
	work->rnd_rot.vx = 256;
	work->rot_x_flag = 1;
    }
    else if ( work->rnd_rot.vx < -256 ) {
	work->rnd_rot.vx =-256;
	work->rot_x_flag = 2;
    }


    // Ｙ軸回転
    switch ( work->rot_y_flag ) {
    case 0:
	work->rnd_rot.vy += irnd()%32-16;
	break;
    case 1:
	work->rnd_rot.vy += irnd()%32-24;
	if ( work->rnd_rot.vy <= 64 ) {
	    work->rot_y_flag = 0;
	}
	break;
    case 2:
	work->rnd_rot.vy += irnd()%32-8;
	if ( work->rnd_rot.vy >=-64 ) {
	    work->rot_y_flag = 0;
	}
	break;
    }

    if ( work->rnd_rot.vy > 256 ) {
	work->rnd_rot.vy = 256;
	work->rot_y_flag = 1;
    }
    else if ( work->rnd_rot.vy < -256 ) {
	work->rnd_rot.vy =-256;
	work->rot_y_flag = 2;
    }


    DG_SetPos( work->mat );
    DG_RotatePos( &work->rnd_rot );
    DG_GetPos( &mattmp );

    
      
    /* 算出した個数分煙生成 */
    for ( i = 0 ; i < n ; i++ ) {

		pos     = &work->prim->pos[clock][work->id];
		uvrgbwh = &( (DG_PRIM2_UVRGBWH *)work->prim->uvrgb[clock] )[work->id];
		param   = &work->param[work->id];

		DG_COPY_VEC( pos, &prevpos );

		_sceVu0ScaleVector( &vectmp, &unit_vec, (float)(i+1) );
		_sceVu0AddVector( pos, pos, &vectmp );
		pos->vx += work->size / 4.f * frnd();
		pos->vy += work->size / 4.f * frnd();
		pos->vz += work->size / 4.f * frnd();
	
		_sceVu0ScaleVector( &param->vec, (FVECTOR *)mattmp.m[2], work->init_speed );
		param->vec.vy += work->rising_speed * (float)(i+1) / (float)n;

		param->radius = work->init_size * ( 1.20f + frnd() * 0.40f );
		angle = TPI * frnd();
		param->cos = vu0_Cos( angle );
		param->sin = vu0_Sin( angle );

		uvrgbwh->w = param->radius * param->cos;
		uvrgbwh->h = param->radius * param->sin;
	
		uvrgbwh->a = work->alpha + irnd()%5;

		if ( ++work->id >= work->num ) {
			work->loop_flag = 1;
			work->id = 0;
		}

    }

    }
    

    work->cnt++;

    /*if ( work->cnt >= 900 ) {
	GV_DestroyActor( work );
    }*/
	

}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
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


	for ( i = 0 ; i < work->num ; i++ ){

	    DG_COPY_VEC( pos, (FVECTOR *)work->mat->m[3] );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = COLOR * ( frnd()*0.2f + 0.8f );
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

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          work->num );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          work->num );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), work->num );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), work->num );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources(
			Work 	*work,
			float 	init_speed,
			float 	rising_speed,
			float 	init_size,
			float 	last_size,
			int	alpha
)
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->id   = 0;    
    work->cnt  = 0;
    work->life = LIFE;
    work->loop_flag = 0;

    work->size = init_size;
    work->init_size = init_size;
    work->last_size = last_size;


    work->init_speed   = init_speed;
    work->rising_speed = rising_speed;    

    work->alpha = alpha;
	
    work->rnd_rot.vx = 0;
    work->rnd_rot.vy = 0;
    work->rnd_rot.vz = 0;

    work->rot_x_flag = 0;
    work->rot_y_flag = 0;



    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->num/8, 8 );

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}


/*-------- void *NewDemoRisingSmoke( FVECTOR* pos, float size ) ----------
                          デモ用汎用立ち昇り煙
  FMATRIX 	*mat,			// 発生マトリクス
  float 	init_speed,		// 初期スピード
  float 	rising_speed,		// 上昇加速度
  float 	init_size,		// 発生サイズ	
  float 	last_size,		// 終了サイズ
  int		alpha,			// アルファ
  int		num			// スプライト数
------------------------------------------------------------------------*/  
void *NewDemoRisingSmoke(
	FMATRIX *mat,			// 発生マトリクス
	float 	init_speed,		// 初期スピード
	float 	rising_speed,		// 上昇加速度
	float 	init_size,		// 発生サイズ	
	float 	last_size,		// 終了サイズ
	int	alpha,			// アルファ
	int	num			// スプライト数
)
{
	Work	*work;
	int	number;

	number = (num/8)*8;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, ( sizeof( Work ) + sizeof( SMOKE_PARAM )*number ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->num = number;

		work->mode = DEMO_SMOKE_LINK;
		work->mat = mat;

		if ( GetResources( work, init_speed, rising_speed, init_size, last_size, alpha ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



void *NewDemoRisingSmokeDummy(
	int	con_name,			// エフェクトコントロール名			      
	float 	init_speed,		// 初期スピード
	float 	rising_speed,		// 上昇加速度
	float 	init_size,		// 発生サイズ	
	float 	last_size,		// 終了サイズ
	int	alpha,			// アルファ
	int	num			// スプライト数
)
{
	Work	*work;
	int	number;

	number = (num/8)*8;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, ( sizeof( Work ) + sizeof( SMOKE_PARAM )*number ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->num = number;

		work->mode = DEMO_SMOKE_DUMMY;
		work->eft_ctrl = DM_GetEftControl( con_name );
		DM_EftControlMatrix( work->eft_ctrl, &work->matrix );
		work->mat = &work->matrix;

		if ( GetResources( work, init_speed, rising_speed, init_size, last_size, alpha ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


void *NewDemoRisingSmokeFix(
	FVECTOR *pos,			// 位置
	SVECTOR *rot,			// 回転
	float 	init_speed,		// 初期スピード
	float 	rising_speed,		// 上昇加速度
	float 	init_size,		// 発生サイズ	
	float 	last_size,		// 終了サイズ
	int	alpha,			// アルファ
	int	num			// スプライト数
)
{
	Work	*work;
	int	number;

	number = (num/8)*8;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, ( sizeof( Work ) + sizeof( SMOKE_PARAM )*number ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );		
		work->num = number;

		work->mode = DEMO_SMOKE_FIX;
		DG_SetPos2( pos, rot );
		DG_GetPos( &work->matrix );
		work->mat = &work->matrix;

		if ( GetResources( work, init_speed, rising_speed, init_size, last_size, alpha ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}

