//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_missile_barn.c
    ミサイル分裂燃え
    2001/04/25 Yuuta Kunibe	
    $Id: solidus_missile_barn.c,v 1.1.1.3 2002/11/19 11:44:50 Yoshizawa1 Exp $
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




#define N_PRIMS		(4)
#define	N_VERTS		(16)
#define	N_SPRT		(N_PRIMS*N_VERTS)

#define	FIRE_TEX	( GV_StrCode( "blood_2bw_msk" ) )

#define	COL_R		(128)
#define	COL_G		(24)
#define	COL_B		(8)
#define ALPHA		(20)

#define ALPHA_MIN	(64)
#define	ALPHA_WIDTH	(8)

#define	FIRE_MAX_SIZE	(80.0f)
#define	FIRE_MIN_SIZE	(20.0f)

#define	LIFE		(300)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define WIND_MAX 	(20.f)

#define	SPRAY_LENGTH	(3000.0f)



enum {
    BODY,
    L_ARM,
    R_ARM,
    L_LEG,
    R_LEG,
};


typedef struct {

    FVECTOR	vec;
    float	rad;
    float	rot;
    float	rot_add;

    int		node;
    float	param;
    float	rnd_x;
    float	rnd_z;

} FIRE_PARAM;


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FIRE_PARAM	param[N_SPRT];

    FMATRIX	*mat;
    
    float	size;
    float	radius;

    float	initsize;
    float	rising_spd;
    FVECTOR	wind;

    int		ext_flag;		/* 消火フラグ */
    int		proc;			/* 消火プロック */

    int 	life;
    int		cnt;

} Work;




static void ActFire( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FIRE_PARAM		*param;

    float		ftmp;
    float		cos;

    FVECTOR		vectmp;

    int			alpha_flag;

    int			invisible_num;




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
    param 	= work->param;


    invisible_num = 0;


    if ( !( work->cnt%4 ) ) {
	alpha_flag = 1;
    }
    else {
	alpha_flag = 0;
    }



    for ( i = 0 ; i < N_SPRT ; i++ ) {	

	/* スプライト更新 */
	if ( uvrgbwh_pre->a > ALPHA_MIN ) {

	    /* 本体付随炎 */
	    if ( i%3 ) {

		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->mat->m[1], param->param * 200.0f );
 		_sceVu0AddVector( pos, (FVECTOR *)work->mat->m[3], &vectmp );

	    }
	    /* 舞い上がる炎 */
	    else {

		/* 位置更新 */
		_sceVu0AddVector( pos, pos_pre, &param->vec );	    

		/* 速度更新 */
		param->vec.vx = 12.5f * frnd();
		param->vec.vz = 12.5f * frnd();

	    }

	    /* 幅,高さ更新 */
	    cos = vu0_Cos( param->rot );
	    uvrgbwh->w = (short)( param->rad * cos );
	    uvrgbwh->h = (short)( param->rad * cos );


	    /* 角度更新 */
	    param->rot += param->rot_add;
	    if ( param->rot > PI ) {
		param->rot -= TPI;
	    }
	    else if ( param->rot < -PI ) {
		param->rot += TPI;
	    }


	    if ( uvrgbwh->a > ALPHA_MIN + 5 ) {

		param->rad *= 0.95f;

		if ( !work->ext_flag ) {
		    param->vec.vy += 0.30f;
		}

		uvrgbwh->a = uvrgbwh_pre->a - 1;

	    }
	    else {

		param->rad *= 0.50f;
		if ( !work->ext_flag ) {
		    param->vec.vy += 0.10f;
		}
		uvrgbwh->a = uvrgbwh_pre->a - 2;

	    }

	}
	/* スプライト初期化 */
	else {

		param->param = 0.5f + 0.5f * frnd();

		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->mat->m[1], param->param * 200.0f );
 		_sceVu0AddVector( pos, (FVECTOR *)work->mat->m[3], &vectmp );

		/* パラメータ初期化 */
		ftmp = ( 0.5f + frnd() * 0.50f );
		param->rad = work->size * ( 0.5f + ftmp );
		param->rot = PI / 2.0f;

		param->vec.vx = work->rising_spd * 25.0f / 30.0f * frnd();
		param->vec.vy = work->rising_spd * ( 1.0f - ftmp ) / 1.0f + work->rising_spd * 2.f / 3.f; 
		param->vec.vz = work->rising_spd * 25.0f / 30.0f * frnd();
		_sceVu0ScaleVector( &param->vec, &param->vec, 0.50f );

		/* 幅,高さ,アルファ初期化 */
		cos = vu0_Cos( param->rot );
		uvrgbwh->w = (short)( param->rad * cos );
		uvrgbwh->h = (short)( param->rad * cos );
		uvrgbwh->a = ALPHA_MIN + irnd()%ALPHA_WIDTH;

	}

	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	param++;
	
    }

}







/* アクト関数 */
static void Act( Work *work )
{

    ActFire( work );
    work->cnt++;

}




static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );

}




/* 炎初期化関数 */
static int InitPacket( Work *work )
{

	int			i;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FIRE_PARAM		*param;



	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		return 0;
	}

	tex = DG_GetTexture( FIRE_TEX );


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	param	= work->param;

	for ( i = 0 ; i < N_SPRT ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    switch ( irnd()%4 ) {
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

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G + irnd()%COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = 32 + irnd()%16;

	    param->rad = work->size * ( 1.0f + frnd() * 0.50f );
	    param->rot = TPI * frnd();
	    param->rot_add = TPI * 0.50f * frnd();//TPI * 0.1f * frnd();
	    uvrgbwh->w = param->rad * vu0_Cos( param->rot );
	    uvrgbwh->h = param->rad * vu0_Sin( param->rot );

	    DG_COPY_VEC( &param->vec, &DG_ZeroVector );
	    param->vec.vx = 100.f * frnd();
	    param->vec.vy = 150.f * frnd() + 200.f; 
	    param->vec.vz = 100.f * frnd();

	    pos++;
	    uvrgbwh++;
	    param++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRT );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRT );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRT );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRT );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}




static int GetResources( Work *work, FMATRIX *mat )
{

    work->mat = mat;

    work->life = LIFE;
    work->cnt  = 0;
    work->ext_flag = 0;

    work->size       = FIRE_MAX_SIZE;
    work->radius     = 200.0f;
    work->rising_spd = 10.0f;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    if ( InitPacket( work ) ) {	   
		return 0;
	}
	else {
		return -1;
	}

}



void *NewSolidusMissileBarn( FMATRIX *mat )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



