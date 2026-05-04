//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_claster_smoke.c
    クラスター爆弾分裂煙
    2001/04/05 Yuuta Kunibe	
    $Id: harrier_claster_smoke.c,v 1.1.1.3 2002/11/19 11:44:40 Yoshizawa1 Exp $
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


#define N_PRIMS		(8)
#define	N_VERTS		(8)
#define	N_SPRTS		(N_PRIMS*N_VERTS)

#define BASE_SIZE	(2000.f)

#define COL_R		(16)
#define COL_G		(16)
#define COL_B		(16)
#define ALPHA		(48)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	vec[N_SPRTS];
    int 	counter[N_SPRTS];

    int		cnt;

} Work;

/*-------- 汎用火花  --------*/
extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count ) ;


/* アクト関数 */
static void Act( Work *work )
{

    int		i;
    int	  	clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;

    FVECTOR		*vec;

    int			invisible_count;


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
    vec 	= work->vec;

    invisible_count = 0;
    
    /* スプライト更新 */
    for ( i = 0 ; i < N_SPRTS ; i++ ) {

	_sceVu0AddVector( pos, pos_pre, vec );
	if ( work->counter[i] > 0 ) {
	    work->counter[i]--;
	}
	else {
	    _sceVu0ScaleVector( vec, vec, 0.95f );
	}

	if ( uvrgbwh_pre->a > 0 ) {
	    uvrgbwh->w = (short)( (float)uvrgbwh_pre->w * 1.05f );
	    uvrgbwh->h = (short)( (float)uvrgbwh_pre->h * 1.05f );
	    uvrgbwh->a = uvrgbwh_pre->a - 1;
	}
	else {
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	    uvrgbwh->a = 0;

	    invisible_count++;
	}
	    	
	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;

    }

    if ( invisible_count >= 64 ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR *position, FVECTOR *vector, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float			size;
	float 			ftmp;
	float			angle;

	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;	

	for ( i = 0 ; i < N_SPRTS ; i++ ) {

	    DG_COPY_VEC( pos, position );
	    pos->vx += frnd()*2500.f;
	    pos->vy += frnd()*2500.f;
	    pos->vz += frnd()*2500.f;

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
	    uvrgbwh->a  = ALPHA + irnd()%10 - 5;

	    angle  = frnd() * TPI;
	    ftmp = 0.6f + 0.4f * frnd();
	    size = BASE_SIZE * 0.25f + BASE_SIZE * ftmp; 
	    uvrgbwh->w  = (short)( size * vu0_Cos( angle ) );
	    uvrgbwh->h  = (short)( size * vu0_Sin( angle ) );

	    DG_COPY_VEC( vec, vector );
	    vec->vx += 1000.f * frnd(); 
	    vec->vy += 1000.f * frnd(); 
	    vec->vz += 1000.f * frnd();

	    work->counter[i] = irnd()%10;

	    pos++;
	    uvrgbwh++;
	    vec++;

	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, FVECTOR *position, FVECTOR *vector )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;


    work->cnt = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, position, vector, prim, tex );    


    /* 火花呼び出し */
    {
	FMATRIX		mat;
	SVECTOR		rot;
	SVECTOR		rot_wide;
	FVECTOR		color;
	FVECTOR		vectmp;


	_sceVu0Normalize( &vectmp, vector );
	OK_DirVecXY( &DG_ZeroVector, &vectmp, &rot );
	rot.vx += -256-512;
	rot.vy += 2048-512;
	       	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	_sceVu0ScaleVector( &vectmp, &vectmp, 3000.0f );
	_sceVu0AddVector( (FVECTOR *)mat.m[3], position, &vectmp );

	rot_wide.vx = 1024;
	rot_wide.vy = 1024;
	rot_wide.vz = 4096;

	color.vx = 255.0F;
	color.vy = 128.0F;
	color.vz = 32.0F;
	color.vw = 50.0F;

	NewSpark2( 128,					/* 発生火花数 */
		   &mat,				/* マトリクス */
		   1.0F,				/* 最小スピード */
		   1000.0F + 100.f * frnd(),		/* 幅スピード */
		   10.0F,				/* 重力 */
		   &rot, &rot_wide,			/* 回転 回転幅 */
		   &color,				/* 色 */
		   1.0F,				/* スピードに対する火の長さの割合 */
		   30 );				/* 生存フレーム数 */
    }
    
    
    return 0;

}


/*
  NewHarrierPodSmoke : 呼びだし関数 
  FVECTOR *position : 発生位置
  float   *vector   : ミサイル速度ベクトル
 */
void *NewHarrierClasterSmoke( FVECTOR *position, FVECTOR *vector )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, position, vector ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


