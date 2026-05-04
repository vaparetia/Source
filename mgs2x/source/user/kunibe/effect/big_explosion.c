//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    missile_exp.c
    汎用ミサイル爆発
    2001/04/17 Yuuta Kunibe	
    $Id: big_explosion.c,v 1.1.1.3 2002/11/19 11:44:32 Yoshizawa1 Exp $
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


#define COL_R		(64)
#define COL_G		(64)
#define COL_B		(64)

#define ALPHA		(64)

#define N_PRIMS		(128)
#define	LIFE		(180)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim_exp;

    FVECTOR	center;
    
    float	rot[N_PRIMS];
    float	rot_add[N_PRIMS];
    float	rad[N_PRIMS];
    float	g[N_PRIMS];
    float	cos[N_PRIMS];
    float	sin[N_PRIMS];

    int		id[N_PRIMS];
    FMATRIX	mat[8];
    
    int		cnt;
    FVECTOR	next;

    float	width_param;
    float	rising_speed;


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

    FVECTOR		vectmp;
    float		size;
    float		radius;
    int buffSwitch;

    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_FULL
    buffSwitch = DG_SwitchBuffPrim2( work->prim_exp );
    clock = work->prim_exp->buffer_clock;

    pos		= work->prim_exp->pos[clock];
    pos_pre	= work->prim_exp->pos[buffSwitch ^ clock];
    uvrgbwh	= work->prim_exp->uvrgb[clock];
    uvrgbwh_pre = work->prim_exp->uvrgb[buffSwitch ^ clock];
    
    for ( i = 0 ; i < 8 ; i++ ) {
	work->mat[i].m[3][1] += work->rising_speed;
    }

    if ( work->width_param < 2.50f ) {
	work->width_param += 0.10f;// 0.06f
	size   = 600.0f + ( 3000.0f - 600.0f ) * ( work->width_param - 1.0f ) / 1.50f;
	radius = 200.0f + ( 1000.0f - 200.0f ) * ( work->width_param - 1.0f ) / 1.50f;
	work->rising_speed *= 0.950f;
    }
    else {
	work->width_param += 0.02f;//= 2.5f;
	size   = 3000.0f;
	radius = 1000.f;
	work->rising_speed *= 0.97f;
    }
    
	
    
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	vectmp.vx = radius * work->width_param * vu0_Cos( work->rot[i] );
	vectmp.vy = radius * vu0_Sin( work->rot[i] );
	vectmp.vz = 0.f;
	vectmp.vw = 0.f;

	if ( vectmp.vx < 0.f ) vectmp.vx = 0.f;
	
	DG_SetPos( &work->mat[work->id[i]] );
	DG_PutVector( &vectmp, pos, 1 );

	work->rot[i] += work->rot_add[i];
	//if ( work->rot[i] < -PI ) work->rot[i] += TPI;
	if ( work->rot[i] < -PI/2.f ) work->rot[i] += PI;

	uvrgbwh->w = size * work->cos[i];
	uvrgbwh->h = size * work->sin[i];


	if ( work->cnt > 32 ) {
	    uvrgbwh->g = (u_short)( (float)uvrgbwh_pre->g * 0.72f + work->g[i] * 0.28f );
	    uvrgbwh->b = (u_short)( (float)uvrgbwh_pre->b * 0.72f );
	    if ( uvrgbwh_pre->a > 2 ) {
		uvrgbwh->a = uvrgbwh_pre->a - 2;
	    }
	    else {
		uvrgbwh->w = 0;
		uvrgbwh->h = 0;
		uvrgbwh->a = 0;
	    }
	}
	else if ( work->cnt > 16 ) {
	    uvrgbwh->g = (u_short)( (float)uvrgbwh_pre->g * 0.8f + work->g[i] * 0.2f );
	    uvrgbwh->b = (u_short)( (float)uvrgbwh_pre->b * 0.8f );
	}
		
	pos++;
	uvrgbwh++;
	uvrgbwh_pre++;
    }		

    printf("cnt %d\n", work->cnt);
    
    if ( ++work->cnt >= 120 ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim_exp = OK_FreePrim2( work->prim_exp );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR* center )
{
	int			i;

	DG_PRIM2		*prim;

	DG_TEX			*tex;

	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float			size;
	float			angle;
	SVECTOR			matrot;


	size = 500.f;

	/* 爆発コア初期化 */
	prim = work->prim_exp = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
	tex = DG_GetTexture( GV_StrCode( "bombgas1_alp" ) );
	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i=0; i<8; i++ ) {
	    matrot.vx = 0;
	    matrot.vy = ( 4096 / 8 ) * i;
	    matrot.vz = 0;
	    DG_SetPos2( center, &matrot );
	    DG_GetPos( &work->mat[i] );
	}
	
	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    DG_COPY_VEC( pos, center );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    angle = TPI*frnd();
	    work->cos[i] = vu0_Cos( angle );
	    work->sin[i] = vu0_Sin( angle ); 
	    uvrgbwh->w  = 600.f * work->cos[i];//3000.f * work->cos[i];
	    uvrgbwh->h  = 600.f * work->sin[i];//3000.f * work->sin[i];

	    uvrgbwh->r  = 110 + irnd()%18;
	    uvrgbwh->g  = 32 + irnd()%52;
	    uvrgbwh->b  = irnd()%16;
	    uvrgbwh->a  = ALPHA * ( 0.75f + frnd()*0.25f );

	    work->g[i] = 32.f + frnd() * 16.f;

	    work->rad[i] = 200.f;
	    //work->rot[i] = TPI*frnd();
	    work->rot[i] = PI*frnd() + PI/2.f;
	    work->rot_add[i] =-TPI / 120.f;

	    work->id[i] = i%8;
	    
	    pos++;
	    uvrgbwh++;

	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, FVECTOR* center )
{
    
    work->cnt = 0;
    work->next = *center;
    work->width_param = 1.f;
    work->rising_speed = 300.0f;

    DG_COPY_VEC( &work->center, center );
    
    InitPacket( work, center );    

    return 0;

}


void *NewBigExplosion( FVECTOR* center )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		if ( GetResources( work, center ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


