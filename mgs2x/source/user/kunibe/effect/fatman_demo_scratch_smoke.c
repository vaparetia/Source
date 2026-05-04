//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    fatman_scratch_smoke.c
    ファットマンスピン煙
    2001/07/15 Yuuta Kunibe	
    $Id: fatman_demo_scratch_smoke.c,v 1.1.1.3 2002/11/19 11:44:38 Yoshizawa1 Exp $
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


#define	N_PRIMS	(8)
#define	N_VERTS	(8)
#define	N_SPRTS	(N_PRIMS*N_VERTS)

#define COLOR	(64)//(128)
#define SIZE	(100.0f)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define	INIT_SPEED		(50.0f)
#define	RISING_SPEED		(0.5f)//(1.0f)
#define	INIT_SIZE		(250.0f)
#define	LAST_SIZE		(2000.0f)
#define	ALPHA			(4.0f)

//#define WIND_MAX (10.f)


/* グローバル風パラメータ */
/*extern FVECTOR	G_wind;
extern SVECTOR	G_wind_rot;
extern int	G_wind_intense;
extern int	G_wind_intense_max;
extern FMATRIX	G_wind_matrix;*/
extern float	OK_slow_param;


typedef struct {

    FVECTOR	vec;
    float	radius;
    float	cos;
    float	sin;

} SMOKE_PARAM;


typedef	struct	{

    GV_ACT_EX	actor;
    int 	name;
    int 	code[2];

    DG_PRIM2	*prim;

    FMATRIX	*mat;
    FVECTOR	vec;
    FVECTOR	pre_pos;

    float	size;
    float	init_size;
    float	last_size;
    float	rising_speed;
    float	alpha;

    float	init_speed;
    //FVECTOR	wind;

    /* 発生ベクトル振り用 */
    SVECTOR	rnd_rot;
    int		rot_x_flag;
    int		rot_y_flag;

    int  	id;
    int		cnt;
    int		loop_flag;

    int 	num;
    int		flag;

    float	add_speed;
    float	add_alpha;
    
    
    SMOKE_PARAM	param[N_SPRTS];    

} Work;


static void RecieveMessage( Work* work )
{
    
    GV_MSG*	msg;
    int 	n_msg;
    int		time;


    if ( work->name == 0 ) {
	return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:
	    work->flag = 1;
	    time = msg->message[1];
	    work->add_alpha = work->alpha / (float)time;
	    work->add_speed = work->init_speed / (float)time;
	    break;
	}
	msg++;
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
    //float		intense;
    float		len;
    float		ftmp;
    float		angle;
    FVECTOR		vectmp;
    FVECTOR		prevpos;
    FVECTOR		unit_vec;
    //FVECTOR		local_wind;
    int			alpha;
    FMATRIX		mattmp;

    float		rate;
    GV_MSG msg;
    
    SMOKE_PARAM		*param;



    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];

    param = work->param;


    /* 風計算 ->> ファットマンステージは風がないみたいだからとりあえずカット */
    /*intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;*/




    if ( GV_PadData[1].press & PAD_X ) {
	work->code[0] = 0;
	work->code[1] = 60;

	msg.address = work->name;
	msg.message = work->code;
	msg.message_len = 2;
	GV_SendMessage( &msg );
    }




    _sceVu0SubVector( &work->vec, (FVECTOR *)work->mat->m[3], &work->pre_pos );
    if ( _sceVu0InnerProduct( &work->vec, &work->vec ) > 0.0f ) {
	_sceVu0Normalize( &work->vec, &work->vec );
    }
	

    n = work->id - 1;
    if ( n < 0 ) n = N_SPRTS - 1;

    /* スローパラメータ対応デバッグ */
    /*if ( GV_PadData[1].press & PAD_L2 ) {
	if ( OK_slow_param == 1.0f) {
	    OK_slow_param = 0.0f;
	}
	else {
	    OK_slow_param = 1.0f;
	}
    }*/

    RecieveMessage( work );

    /* スプライト更新 */
    for ( i = 0 ; i < N_SPRTS ; i++ ) {	
	
	/*ftmp = 1.0f + frnd() * 0.2f;
	_sceVu0ScaleVector( &vectmp, &work->wind, ftmp );
	_sceVu0AddVector( pos, pos_pre, &vectmp );*/

	_sceVu0ScaleVector( &vectmp, &param->vec, OK_slow_param );
	//_sceVu0AddVector( pos, pos, &vectmp );
	_sceVu0AddVector( pos, pos_pre, &vectmp );	/* 風入れた時に変更 */
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
	if ( j < 0 ) j += N_SPRTS;

	if ( work->loop_flag ) {
	    alpha = work->alpha * ( N_SPRTS - j ) / N_SPRTS;	
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
	    ftmp = param->radius * (float)( N_SPRTS - j ) / (float)N_SPRTS
		+ work->last_size * (float)j / (float)N_SPRTS;
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




    /* 煙初期化 */
    if ( !work->flag ) {
	
	/* 前の煙との直線距離から発生させる煙の数を算出 */
	_sceVu0SubVector( &vectmp, &prevpos, (FVECTOR *)work->mat->m[3] );
	len = bp_sqrtf( _sceVu0InnerProduct( &vectmp, &vectmp ) );  //BP_MATH - emulate PS2 sqrtf
	
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

	
	/* 煙発生の隙間があれば煙生成 */
	if ( n > 0 ) {
	    
	    ftmp = 1.0f / (float)n;
	    _sceVu0SubVector( &unit_vec, (FVECTOR *)work->mat->m[3], &prevpos );
	    _sceVu0ScaleVector( &unit_vec, &unit_vec, ftmp );
	    
	    work->init_size = work->init_size * ( 1.f + frnd()*0.1f );
	    
	    if (!(irnd()%120)) work->init_size = work->size * 1.25f;
	    
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
		
		//_sceVu0ScaleVector( &param->vec, (FVECTOR *)mattmp.m[2], work->init_speed );
		_sceVu0ScaleVector( &param->vec, &work->vec, -work->init_speed );
		param->vec.vy += work->rising_speed * (float)(i+1) / (float)n;

		param->radius = work->init_size * ( 1.20f + frnd() * 0.40f );
		angle = TPI * frnd();
		param->cos = vu0_Cos( angle );
		param->sin = vu0_Sin( angle );
		
		uvrgbwh->w = (short)( param->radius * param->cos );
		uvrgbwh->h = (short)( param->radius * param->sin );
		
		uvrgbwh->a = (u_short)work->alpha + irnd()%5;

		if ( ++work->id >= N_SPRTS ) {
		    work->loop_flag = 1;
		    work->id = 0;
		}

	    }
	    
	}

    }



    /* 煙パラメータ減衰 */
	if ( work->flag ) {
	    
	    work->init_speed -= work->add_speed;
	    if ( work->init_speed < 0.0f ) {
		work->init_speed = 0.0f;
	    }

	    work->rising_speed -= 0.01f;
	    if ( work->rising_speed < 0.0f ) {
		work->rising_speed = 0.0f;
	    }
    
	    work->alpha -= work->add_alpha;
	    if ( work->alpha < 0.0f ) {
		work->alpha = 0.0f;
	    }

	}


#if 0    
    printf("work->id %d, init : %f, rise : %f, alpha %f\n",
	   work->id, work->init_speed, work->rising_speed, work->alpha );
#endif


    /* 位置確保 */
    DG_COPY_VEC( &work->pre_pos, (FVECTOR *)work->mat->m[3] );
	
    work->cnt++;


    if ( work->alpha <= 0.0f ) {
	GV_DestroyActor( work );
    }
	

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

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;


	for ( i = 0 ; i < N_SPRTS ; i++ ){

	    DG_COPY_VEC( pos, (FVECTOR *)work->mat->m[3] );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = COLOR * ( frnd() + 1.f );
	    uvrgbwh->r  = ftmp;
	    uvrgbwh->g  = ftmp;
	    uvrgbwh->b  = ftmp;
	    uvrgbwh->a  = 0;

	    uvrgbwh->w  = 0;
	    uvrgbwh->h  = 0;

	    pos++;
	    uvrgbwh++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources(
			Work 	*work,
			FMATRIX *mat )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->id   = 0;    
    work->cnt  = 0;
    work->loop_flag = 0;

    work->mat  = mat;
    DG_COPY_VEC( &work->pre_pos, (FVECTOR *)work->mat->m[3] );

    work->size         = INIT_SIZE;
    work->init_size    = INIT_SIZE;
    work->last_size    = LAST_SIZE;
    work->init_speed   = INIT_SPEED;
    work->rising_speed = RISING_SPEED;    

    work->alpha = ALPHA;

    work->rnd_rot.vx = 0;
    work->rnd_rot.vy = 0;
    work->rnd_rot.vz = 0;

    work->rot_x_flag = 0;
    work->rot_y_flag = 0;

    /* ファットマンステージは風ないみたいだからとりあえずカット */
    //DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}


/*-------- void *NewFatmanScratchSmoke( FMATRIX *mat, int *flag ) ----------
                          ファットマンスピン煙
  FMATRIX 	*mat,			// 発生マトリクス
  int		*flag			// スピンフラグ
------------------------------------------------------------------------*/  
void *NewFatmanDemoScratchSmoke(
	FMATRIX *mat,
	int	name )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



