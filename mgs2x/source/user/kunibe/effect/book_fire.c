//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    c_box_fire.c
    電撃床用ダンボール燃え
    2001/04/25 Yuuta Kunibe	
    $Id: book_fire.c,v 1.1.1.3 2002/11/19 11:44:34 Yoshizawa1 Exp $
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




#define N_PRIMS		(12)
#define	N_VERTS		(16)
#define	N_SPRT		(N_PRIMS*N_VERTS)

#define	FIRE_TEX	( GV_StrCode( "blood_2bw_msk" ) )

#define	COL_R		(72)
#define	COL_G		(24)
#define	COL_B		(16)
#define ALPHA		(20)

#define ALPHA_MIN	(80)
#define	ALPHA_WIDTH	(16)

#define	FIRE_MAX_SIZE	(300.0f)
#define	FIRE_MIN_SIZE	(100.0f)

#define	LIFE		(150)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)


#define	N_PRIMS2	(2)
#define	N_VERTS2	(16)
#define	N_POWDER	(N_PRIMS2*N_VERTS2)

#define	COL_POWDER	(128)
#define	POWDER_ALPHA	(32)
#define	POWDER_ALPHA_WIDTH	(16)

#define	POWDER_TEX	( GV_StrCode( "bombpowder7_msk" ) )


static int BOOK_FIRE_SE[] = {
    SD_A_FIREBOK1,
    SD_A_FIREBOK2,
};


typedef struct {

    FVECTOR	vec;
    float	rad;
    float	rot;
    float	rot_add;

} FIRE_PARAM;


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FIRE_PARAM	param[N_SPRT];

    FMATRIX	mat;
    FVECTOR	bound_max;
    FVECTOR	bound_min;
    
    float	size;
    float	radius;

    float	initsize;
    float	rising_spd;

    int 	life;
    int		cnt;
    
    DG_PRIM2	*prim_powder;
    FVECTOR	vec_powder[N_POWDER];
    int		id;

    FVECTOR	smoke_pos;
    float	rate;
    int		smoke_count;

    int		se_id;
    int 	se_count;
    
} Work;




static int ActFire( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FIRE_PARAM		*param;

    float		rnd;
    float		ftmp;
    float		cos;

    FVECTOR		vectmp;
    FVECTOR		vectmp2;

    int			alpha_flag;

    int			invisible_num;



    /* 操作する頂点バッファ取得 */
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

	    /* 位置更新 */
	    _sceVu0AddVector( pos, pos_pre, &param->vec );

	    /* 速度更新 */
	    param->vec.vx = 12.5f * frnd();
	    param->vec.vz = 12.5f * frnd();

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
		param->rad *= 0.985f;
		param->vec.vy += 0.30f *work->rate*2.0f;
		if ( alpha_flag ) {
		    uvrgbwh->a = uvrgbwh_pre->a - 1;
		}
		else {
		    uvrgbwh->a = uvrgbwh_pre->a;
		}
	    }
	    else {
		param->rad *= 0.50f;
		param->vec.vy += 0.10f *work->rate*2.0f;
		uvrgbwh->a = uvrgbwh_pre->a - 1;
	    }

	}
	/* スプライト初期化 */
	else {

	    if ( i > work->cnt - LIFE ) {

		/* 初期位置設定 */
		//rnd = 0.50f + frnd()*0.50f;		/* 全面version */
		rnd = work->rate + frnd()*work->rate;	/* セル化version */
		vectmp.vx = work->bound_max.vx * rnd + work->bound_min.vx * ( 1.0f - rnd );
		rnd = work->rate + frnd()*work->rate;
		vectmp.vy = work->bound_max.vy * rnd + work->bound_min.vy * ( 1.0f - rnd );
		rnd = work->rate + frnd()*work->rate;
		vectmp.vz = work->bound_max.vz * rnd + work->bound_min.vz * ( 1.0f - rnd );
		vectmp.vw = 1.0f;

		/* 初期位置絶対化 */
		DG_SetPos( &work->mat );
		DG_PutVector( &vectmp, pos, 1 );

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
	    else {
		uvrgbwh->w = uvrgbwh->h = 0;
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



    _sceVu0ScaleVector( &vectmp, &work->bound_max, work->rate );
    _sceVu0ScaleVector( &vectmp2, &work->bound_min, ( 1.0f-work->rate ) );
    _sceVu0AddVector( &vectmp, &vectmp, &vectmp2 );

    DG_SetPos( &work->mat );
    DG_PutVector( &vectmp, &work->smoke_pos, 1 );
    
    

    /* 煙発生 */
    /*if ( work->cnt == work->smoke_count ) {
	extern void *NewBoxSmoke( FVECTOR* pos, float size );
	//NewBoxSmoke( (FVECTOR *)work->mat.m[3], 300.0f );
	NewBoxSmoke( &work->smoke_pos, 300.0f );
    }*/   
    

    if ( work->cnt > LIFE ) {
	if ( work->size > FIRE_MIN_SIZE ) {
	    if ( work->rate > 0.25f ) {
		work->rate -= 0.002f;
	    }
	    work->rising_spd *= 0.99f;
	    work->size *= 0.99f;
	}
    }


    /* SE呼び出し */
    if ( work->rate > 0.40f ) {
        if ( work->se_count <= 0 ) {
	    GM_SeSetMode( BOOK_FIRE_SE[work->se_id],
			  (FVECTOR *)work->mat.m[3],
			  GM_SEMODE_NORMAL );	
	    work->se_count = DIRECT_TICK( 6+irnd()%10 );
	}
	else {
	    work->se_count--;
	}
    }    
    

    if ( invisible_num >= N_SPRT ) {
	return 0;
    }
    else {
	return 1;
    }


}




static int ActPowder( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    float		rnd;
    FVECTOR		vectmp;
    int			invisible_num;


    /* 操作する頂点バッファ取得 */
    clock = work->prim_powder->buffer_clock;

    pos		= work->prim_powder->pos[clock];
    pos_pre	= work->prim_powder->pos[1-clock];
    uvrgbwh	= work->prim_powder->uvrgb[clock];
    uvrgbwh_pre = work->prim_powder->uvrgb[1-clock];
    vec         = work->vec_powder;


    invisible_num = 0;
    

    for ( i = 0 ; i < N_POWDER ; i++ ) {

	if ( uvrgbwh_pre->a > 0 ) {
	    _sceVu0AddVector( pos, pos_pre, vec );

	    pos->vx += vec->vy * 0.50f * frnd();
	    pos->vz += vec->vy * 0.50f * frnd();
		
	    vec->vx *= 0.99f;
	    vec->vz *= 0.99f;

	    uvrgbwh->a = uvrgbwh_pre->a - 1;

	}
	else {
	    invisible_num++;
	    uvrgbwh->a = 0;
	}
	
	pos++;
	pos_pre++;
	vec++;
	uvrgbwh++;
	uvrgbwh_pre++;

    }
	

    if ( work->cnt < 180 ) {

	if ( work->cnt % 2 ) {

	    pos     = &work->prim_powder->pos[clock][work->id];
	    uvrgbwh = &((DG_PRIM2_UVRGBWH *)work->prim_powder->uvrgb[clock])[work->id];
	    vec     = &work->vec_powder[work->id];

	    rnd = 0.50f + frnd()*0.50f;
	    vectmp.vx = work->bound_max.vx * rnd + work->bound_min.vx * ( 1.0f - rnd );
	    rnd = 0.50f + frnd()*0.50f;
	    vectmp.vy = work->bound_max.vy * rnd + work->bound_min.vy * ( 1.0f - rnd );
	    rnd = 0.50f + frnd()*0.50f;
	    vectmp.vz = work->bound_max.vz * rnd + work->bound_min.vz * ( 1.0f - rnd );
	    vectmp.vw = 1.0f;

	    /* 初期位置絶対化 */
	    DG_SetPos( &work->mat );
	    DG_PutVector( &vectmp, pos, 1 );	    
    
	    vec->vy = 100.0f + frnd() * 25.0f;
	    vec->vx += vec->vy * 0.20f * frnd();
	    vec->vz += vec->vy * 0.20f * frnd();       	

	    uvrgbwh->a = POWDER_ALPHA + irnd()%POWDER_ALPHA_WIDTH;

	}

    }
    else if ( work->cnt < LIFE+120 ) {
	
	if ( !( irnd() % ( 2 + (work->cnt-180)/6 ) ) ) {

	    pos     = &work->prim_powder->pos[clock][work->id];
	    uvrgbwh = &((DG_PRIM2_UVRGBWH *)work->prim_powder->uvrgb[clock])[work->id];
	    vec     = &work->vec_powder[work->id];

	    rnd = 0.50f + frnd()*0.50f;
	    vectmp.vx = work->bound_max.vx * rnd + work->bound_min.vx * ( 1.0f - rnd );
	    rnd = 0.50f + frnd()*0.50f;
	    vectmp.vy = work->bound_max.vy * rnd + work->bound_min.vy * ( 1.0f - rnd );
	    rnd = 0.50f + frnd()*0.50f;
	    vectmp.vz = work->bound_max.vz * rnd + work->bound_min.vz * ( 1.0f - rnd );
	    vectmp.vw = 1.0f;

	    /* 初期位置絶対化 */
	    DG_SetPos( &work->mat );
	    DG_PutVector( &vectmp, pos, 1 );	    
    
	    vec->vy = 100.0f + frnd() * 25.0f;
	    vec->vx += vec->vy * 0.20f * frnd();
	    vec->vz += vec->vy * 0.20f * frnd();       	

	    uvrgbwh->a = POWDER_ALPHA + irnd()%POWDER_ALPHA_WIDTH;

	}

    }
        
    if ( ++work->id >= N_POWDER ) {
	work->id = 0;
    }

    if ( invisible_num >= N_POWDER ) {
	return 0;
    }
    else {
	return 1;
    }    
    
    
}
    



/* アクト関数 */
static void Act( Work *work )
{

    int	ret_fire;
    int	ret_powder;

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    /* 炎更新 */
    ret_fire = ActFire( work );
    /* 火の粉更新 */
    ret_powder = ActPowder( work );    


    if ( !ret_fire && !ret_powder ) {
	GV_DestroyActor( work );
    }
    else {    
	work->cnt++;
    }

}




static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
    if ( work->prim_powder ) work->prim_powder = OK_FreePrim2( work->prim_powder );

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
	    uvrgbwh->a  = 32 + irnd()%32;

	    param->rad = work->size * ( 1.0f + frnd() * 0.50f );
	    param->rot = TPI * frnd();
	    param->rot_add = TPI * 0.05f * frnd();//TPI * 0.1f * frnd();
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



/* 火の粉初期化関数 */
static int InitPacket2( Work *work )
{

	int			i;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR			*vec;



	prim = work->prim_powder = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
	if ( prim == NULL ) {
		return 0;
	}

	tex = DG_GetTexture( POWDER_TEX );


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	vec     = work->vec_powder;


	for ( i = 0 ; i < N_POWDER ; i++ ){

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

	    uvrgbwh->r  = COL_POWDER;
	    uvrgbwh->g  = COL_POWDER;
	    uvrgbwh->b  = COL_POWDER;
	    uvrgbwh->a  = 64;

	    uvrgbwh->w = work->size;
	    uvrgbwh->h = work->size;

	    DG_COPY_VEC( vec, &DG_ZeroVector );

	    pos++;
	    uvrgbwh++;
	    vec++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_POWDER );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_POWDER );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_POWDER );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_POWDER );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FMATRIX *mat )
{

    DG_COPY_MAT( &work->mat, mat );

    work->bound_max.vx = 250.0f;
    work->bound_max.vy = 0.0f;
    work->bound_max.vz = 250.0f;

    work->bound_min.vx =-250.0f;
    work->bound_min.vy = 0.0f;
    work->bound_min.vz =-250.0f;


    work->size       = FIRE_MAX_SIZE;
    work->radius     = 200.0f;
    work->rising_spd = 10.0f;

    work->rate = 0.50f;
    work->life = LIFE;
    work->cnt  = 0;
    
    work->smoke_count = irnd()%60;
    
    work->id = 0;

    work->se_id = irnd()%2;
    work->se_count = 0;

    if ( !( InitPacket( work ) ) ) {
		return -1;
	}
    if ( !( InitPacket2( work ) ) ) {
		return -1;
	}

    return 0;

}




void *NewBookFire( FMATRIX *mat )
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




