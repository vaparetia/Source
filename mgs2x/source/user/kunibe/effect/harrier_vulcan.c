//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_vulcan.c
  ハリアーバルカン弾
  2001/04/09 Yuuta Kunibe
  $Id: harrier_vulcan.c,v 1.1.1.3 2002/11/19 11:44:43 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"


#define	N_VERTS		(4)
#define	N_PRIMS		(3)

#define SIZE_X		(25.f)
#define SIZE_Y		(25.f)
#define SIZE_Z		(800.f)

#define BASE_R		(128)
#define BASE_G		(96)
#define BASE_B		(32)
#define ALPHA		(128)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define SPEED		(2000.f)
#define LIFE		(150)


/* 汎用火花エフェクト */
extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count ) ;


typedef	struct	{

    GV_ACT_EX	actor ;

    DG_PRIM2	*prim ;

    float	speed;
    int life;

} Work ;


static void Act( Work *work )
{

    FVECTOR vectmp;
    SVECTOR   rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;


    /* 位置更新 */
    work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&work->prim->as_world.m[2]), SPEED );
    _sceVu0AddVector( (FVECTOR *)(&work->prim->as_world.m[3]), (FVECTOR *)(&work->prim->as_world.m[3]), &vectmp );

    
    if ( work->prim->as_world.m[3][1] <= -1000.f ) {	/* テスト火花呼び出し 実際は当たり取って */
	/* 火花エフェクト呼び出し */
	work->prim->as_world.m[3][1] = -1000.f;
	rot.vx = 3550;
	rot.vy = 0;
	rot.vz = 0;
	rot_wide.vx = 256;
	rot_wide.vy = 256;
	rot_wide.vz = 1 ;
	color.vx = (float)BASE_R;//255.0F ;
	color.vy = (float)BASE_G;//128.0F ;
	color.vz = (float)BASE_B;//128.0F ;
	color.vw = 50.0F ;
	NewSpark2( 24,			/* 発生火花数 */
		   &work->prim->as_world,	/* マトリクス */
		   1.0F,		/* 最小スピード */
		   200.0F,		/* 幅スピード */
		   0.0F,		/* 重力 */
		   &rot, &rot_wide,	/* 回転 回転幅 */
		   &color,		/* 色 */
		   1.0F,		/* スピードに対する火の長さの割合 */
		   15 );		/* 生存フレーム数 */
	GV_DestroyActor( work );
    }
    else if ( ++work->life > LIFE ) {	/* 寿命による解放 */
	GV_DestroyActor( work );
    }       

}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int InitPacket( Work *work, FMATRIX *mat, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i;
	FVECTOR		vectmp;

	DG_ConfigPrim2Tex( prim, tex );		/* テクスチャ指定 */
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	/* 初期位置設定 */
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	DG_COPY_MAT( &prim->as_world, mat );
	DG_COPY_VEC( &vectmp, (FVECTOR const *)(&prim->as_world.m[2]) );
	_sceVu0ScaleVector( &vectmp, &vectmp, 1000.f * frnd() );
	_sceVu0AddVector( (FVECTOR *)(&prim->as_world.m[3]), (FVECTOR *)(&prim->as_world.m[3]), &vectmp );
	
	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	/* XZ平面 */
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vz =-SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vz =-SIZE_Z;
	pos++;

	/* YZ平面 */
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy = SIZE_Y;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy =-SIZE_Y;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy = SIZE_Y;
	pos->vz =-SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy =-SIZE_Y;
	pos->vz =-SIZE_Z;
	pos++;

	/* XY平面 */
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vy = SIZE_Y;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vy = SIZE_Y;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vy =-SIZE_Y;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vy =-SIZE_Y;
	pos++;

	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	return 1;
}

static int GetResources( Work *work, FMATRIX *mat )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->life = 0;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );

	InitPacket( work, mat, prim, tex );

	return 0;

}

void *NewHarrierVulcan( FMATRIX *mat )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
