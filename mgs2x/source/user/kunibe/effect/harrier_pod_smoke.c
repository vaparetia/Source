//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_pod_smoke.c
    ミサイルポッド煙
    2001/04/05 Yuuta Kunibe	
    $Id: harrier_pod_smoke.c,v 1.1.1.3 2002/11/19 11:44:42 Yoshizawa1 Exp $
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


#define BASE_SIZE	(400.f)

#define COL_R		(16)
#define COL_G		(16)
#define COL_B		(16)
#define ALPHA		(50)

#define N_PRIMS		(64)
#define	LIFE		(60)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	vec[N_PRIMS];

    FMATRIX	*mat;
    FVECTOR	vecpod;
    int		cnt;

} Work;


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


    /* マトリクス更新 */
    DG_SetPos( work->mat );
    DG_MovePos( &work->vecpod );
    work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
    DG_GetPos( &work->prim->as_world );


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

    /* スプライト更新 */
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	_sceVu0SubVector( pos, pos_pre, vec );
	_sceVu0ScaleVector( vec, vec, 0.90f );
	
	if ( uvrgbwh_pre->a > 0 ) {
	    if ( i < N_PRIMS / 2 ) {
		uvrgbwh->w = (short)( (float)uvrgbwh_pre->w * 1.03f );
		uvrgbwh->h = (short)( (float)uvrgbwh_pre->h * 1.03f );
	    }
	    uvrgbwh->a = uvrgbwh_pre->a - 1;	    
	}
	else {
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	    uvrgbwh->a = 0;
	}
	    	
	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;
    }

    if ( ++work->cnt >= LIFE ) {
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
	FVECTOR			pos2;

	float			size;
	float 			ftmp;
	float			angle;

	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;	

	/* ポッド後ろ煙 */
	/*_sceVu0ScaleVector( &pos2, vector, 750.f );
	_sceVu0SubVector( &pos2, position, &pos2 );*/

	/**/
	DG_COPY_VEC( &pos2, &DG_ZeroVector );
	pos2.vz = -750.f;
	
	for ( i = 0 ; i < N_PRIMS/2 ; i++ ) {

	    DG_COPY_VEC( pos, &pos2 );

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
	    uvrgbwh->a  = ALPHA + irnd()%7 - 4;

	    angle  = frnd() * TPI;
	    ftmp = 0.6f + 0.4f * frnd();
	    size = BASE_SIZE * 0.25f + BASE_SIZE * ftmp; 
	    uvrgbwh->w  = (short)( size * vu0_Cos( angle ) );
	    uvrgbwh->h  = (short)( size * vu0_Sin( angle ) );

	    ftmp = ftmp * 250.f;
	    /*_sceVu0ScaleVector( vec, vector, ftmp );*/
	    /**/
	    DG_COPY_VEC( vec, &DG_ZeroVector );
	    vec->vz = ftmp;

	    pos++;
	    uvrgbwh++;
	    vec++;
	}

	/* ポッド前煙 */
	/*_sceVu0ScaleVector( &pos2, vector, 750.f );
	_sceVu0AddVector( &pos2, position, &pos2 );*/
	DG_COPY_VEC( &pos2, &DG_ZeroVector );
	pos2.vz = 750.f;

	for ( i = 0 ; i < N_PRIMS/2 ; i++ ) {

	    DG_COPY_VEC( pos, &pos2 );

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
	    uvrgbwh->a  = ALPHA + irnd()%7 - 2;

	    angle  = frnd() * TPI;
	    ftmp = 0.6f + 0.4f * frnd();
	    size = BASE_SIZE * 1.2f * ftmp; 
	    uvrgbwh->w  = (short)( size * vu0_Cos( angle ) );
	    uvrgbwh->h  = (short)( size * vu0_Sin( angle ) );

	    ftmp = ( 1.0f - ftmp ) * 400.f;
	    /*_sceVu0ScaleVector( vec, vector, ftmp );
	    _sceVu0SubVector( vec, &DG_ZeroVector, vec );*/
	    DG_COPY_VEC( vec, &DG_ZeroVector );
	    vec->vz = -ftmp;

	    pos++;
	    uvrgbwh++;
	    vec++;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, FVECTOR *position, FVECTOR *vector, FMATRIX *mat )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;
    FMATRIX	matInv;
    FVECTOR	vectmp;

    work->cnt = 0;

    work->mat = mat;
    _sceVu0SubVector( &vectmp, position, (FVECTOR *)(mat->m[3]) );
    FastInverseMatrix( &matInv, mat );
    matInv.m[3][0] = 0.f;
    matInv.m[3][1] = 0.f;
    matInv.m[3][2] = 0.f;
    _sceVu0ApplyMatrix( &work->vecpod, &matInv, &vectmp );

#if 0    
    printf("vecpod %f, %f, %f\n", work->vecpod.vx, work->vecpod.vy, work->vecpod.vz );
#endif    

    //prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 4, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, position, vector, prim, tex );    

    return 0;

}


/*
  NewHarrierPodSmoke : 呼びだし関数 
  FVECTOR *position : 発生位置
  float   *vector   : ミサイル速度ベクトル
 */
void *NewHarrierPodSmoke( FVECTOR *position, FVECTOR *vector, FMATRIX *mat )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, position, vector, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


