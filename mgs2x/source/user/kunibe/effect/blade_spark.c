//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  blade_spark.c
  刀かち合い光 ( w61a:ソリダス戦 ) ( スローパラメータ対応済 )
  2001/04/20 Yuuta Kunibe	
  $Id: blade_spark.c,v 1.1.1.3 2002/11/19 11:44:33 Yoshizawa1 Exp $
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

#define COL_R	(255)
#define COL_G	(180)
#define COL_B	(96)
#define ALPHA	(128)

#define N_PRIMS	(1)

#define	SIZE	(100.0f)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define	LIFE	(20.0f)

#define	LIT_COL_R	(0xff)
#define	LIT_COL_G	(0x4f)
#define	LIT_COL_B	(0x08)

extern void *NewLightSpark( int n_packets, FMATRIX *matrix,
			    float min_speed, float speed_wide, float gravity,
			    SVECTOR *rot,SVECTOR *rot_wide,
			    FVECTOR *color, float length, int count );

extern float	OK_slow_param;




typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FMATRIX	mat;
    DG_TEX	*tex;
    
    short	w;
    short	h;
    
    float 	count;
    
} Work;


static void Act( Work *work )
{

    
    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    float		near;
    float		far_y;
    int buffSwitch;


    //printf("slow_param : %f\n", OK_slow_param);


    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_FULL
    buffSwitch = DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos	= work->prim->pos[clock];		
    uvrgbwh = work->prim->uvrgb[clock];    
    uvrgbwh_pre = work->prim->uvrgb[buffSwitch ^ clock];    


    if ( OK_slow_param == 1.0f ) {
	if ( uvrgbwh_pre->a >= 24 ) {
	    uvrgbwh->a = uvrgbwh_pre->a - 24;
	    uvrgbwh->w = uvrgbwh_pre->w + work->w;
	    uvrgbwh->h = uvrgbwh_pre->h + work->h;
	    work->w += 20;
	    work->h += 20;
	}
	else {
	    uvrgbwh->a = 0;
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	}
    }
    else {
	if ( uvrgbwh_pre->a >= (int)( 24.0f * OK_slow_param ) ) {
	    uvrgbwh->a = uvrgbwh_pre->a - (int)( 24.0f * OK_slow_param );
	    uvrgbwh->w = uvrgbwh_pre->w + (int)( (float)work->w * OK_slow_param );
	    uvrgbwh->h = uvrgbwh_pre->h + (int)( (float)work->h * OK_slow_param );
	    work->w += (int)( 20.0f * OK_slow_param );
	    work->h += (int)( 20.0f * OK_slow_param );
	}
	else {
	    uvrgbwh->a = 0;
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	}
    }
	

    near = 1000.0f * ( LIFE - work->count ) / LIFE;
    far_y  = 2000.0f * ( LIFE - work->count ) / LIFE;

    DG_SetTmpLight2 (
					 (FVECTOR *)work->mat.m[3],
					 near,
					 far_y,
					 (LIT_COL_R) | (LIT_COL_G)<<8 | (LIT_COL_B)<<16,
					 LIT_FLAG_CHARAONLY );

    work->count += 1.0f * OK_slow_param;    
    
    if ( work->count >= LIFE ) {
		GV_DestroyActor( work );
    }
    
}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, FMATRIX *mat, DG_TEX *tex )
{
    
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	
	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;


	DG_COPY_VEC( pos, (FVECTOR *)mat->m[3] );

	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = COL_R;
	uvrgbwh->g  = COL_G;
	uvrgbwh->b  = COL_B;
	uvrgbwh->a  = ALPHA;

	work->w = uvrgbwh->w  = (int)( SIZE * ( 0.80f + frnd()*0.20f ) );
	work->h = uvrgbwh->h  = (int)( SIZE * ( 0.80f + frnd()*0.20f ) );
	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );


	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );


	return 1;

}



static int GetResources( Work *work, FMATRIX *mat )
{
    
    DG_PRIM2		*prim;

    SVECTOR   rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;

    DG_COPY_MAT( &work->mat, mat );
    work->count = 0.0f;    
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
	if ( prim == NULL ) {
		return (-1);
	}

    work->tex = DG_GetTexture( GV_StrCode( "light05_msk" ) );

    InitPacket( work, prim, mat, work->tex );    

    /* 実際はキャラの中で寿命わけ */
	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 0;
	rot_wide.vx = 4096;
	rot_wide.vy = 4096;
	rot_wide.vz = 4096;
	color.vx = 255.0f;
	color.vy = 96.0f;
	color.vz = 32.0f;
	color.vw = 128.0f;

	NewLightSpark(
		   30,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   40 );		/* 生存フレーム数 */
	NewLightSpark(
		   30,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   45 );		/* 生存フレーム数 */
	NewLightSpark(
		   30,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   50 );		/* 生存フレーム数 */
	NewLightSpark(
		   30,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   55 );		/* 生存フレーム数 */
	NewLightSpark(
		   30,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   60 );		/* 生存フレーム数 */

    return 0;
}


void *NewBladeSpark( FMATRIX *mat )
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


void *NewDemoBladeSpark( FVECTOR *pos )
{
	Work	*work;
	FMATRIX	mat;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		DG_COPY_MAT( &mat, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR *)mat.m[3], pos );
		mat.m[3][3] = 1.0f;
		if ( GetResources( work, &mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





