//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  blade_spark2.c
  刀かち合い光 ( w44a,w45a:天狗兵ステージ )
  2001/06/17 Yuuta Kunibe	
  $Id: blade_spark2.c,v 1.1.1.3 2002/11/19 11:44:33 Yoshizawa1 Exp $
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

#define	SIZE	(100)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define	LIFE	(20)

#define W44A_FLOOR_LEVEL	(4000.0f)
#define	W45A_FLOOR_LEVEL	(0.0f)



extern void *NewLightSpark2( int n_packets, FMATRIX *matrix,
			    float min_speed, float speed_wide, float gravity,
			    SVECTOR *rot,SVECTOR *rot_wide,
			    FVECTOR *color, float length, float height, int count );

extern void *NewBladeSpark( FMATRIX *mat );



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FMATRIX		mat;
    DG_TEX		*tex;

    short		w;
    short		h;
    
    int 		count;
    
} Work;


static void Act( Work *work )
{

    
    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    int buffSwitch;


    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_FULL
    buffSwitch = DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos	= work->prim->pos[clock];		
    uvrgbwh = work->prim->uvrgb[clock];    
    uvrgbwh_pre = work->prim->uvrgb[buffSwitch ^ clock];    


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
    

    DG_SetTmpLight2 (
		     (FVECTOR *)work->mat.m[3],
		     1000.0f * (float)( LIFE - work->count ) / (float)LIFE,
		     2000.0f * (float)( LIFE - work->count ) / (float)LIFE,
		     0xff | 0x4f<<8 | 0x08<<16,
		     LIT_FLAG_CHARAONLY );

    
    if ( ++work->count >= LIFE ) {
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

	work->w = uvrgbwh->w  = SIZE * ( 0.80f + frnd()*0.20f );
	work->h = uvrgbwh->h  = SIZE * ( 0.80f + frnd()*0.20f );
	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );


	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );


	return 1;

}



static int GetResources( Work *work, FMATRIX *mat, float height )
{
    
    DG_PRIM2		*prim;

    SVECTOR   rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;

    work->mat	= *mat;
    work->count = 0;    
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
	if ( prim == NULL ) {
		return -1;
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

	NewLightSpark2(
		   20,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   height,
		   40 );		/* 生存フレーム数 */
	NewLightSpark2(
		   20,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   height,
		   45 );		/* 生存フレーム数 */
	NewLightSpark2(
		   20,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   height,
		   50 );		/* 生存フレーム数 */
	NewLightSpark2(
		   20,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   height,
		   55 );		/* 生存フレーム数 */
	NewLightSpark2(
		   20,			/* 発生火花数 */
		   mat,			/* マトリクス */
		   50.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   10.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   height,
		   60 );		/* 生存フレーム数 */

    return 0;
}


void *NewBladeSpark2( FMATRIX *mat, float height )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, mat, height ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}




/* 刀火花呼び出し口 */
void *NewBladeSparkEffect( FMATRIX *mat, int flag )
{
    switch ( flag ) {
    case 0:	/* flag 0 : w44a */
		return (void *)NewBladeSpark2( mat, W44A_FLOOR_LEVEL );
		break;
    case 1:	/* flag 1 : w45a */
		return (void *)NewBladeSpark2( mat, W45A_FLOOR_LEVEL );
		break;
    case 2:	/* flag 2 : w61a */	
		return (void *)NewBladeSpark( mat );
		break;
    default:
		return NULL;
		break;
    }
}
