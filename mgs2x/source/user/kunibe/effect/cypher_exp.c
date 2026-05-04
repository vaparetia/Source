//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    cypher_exp.c
    サイファー爆発
    2001/06/17 Yuuta Kunibe	
    $Id: cypher_exp.c,v 1.1.1.3 2002/11/19 11:44:35 Yoshizawa1 Exp $
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
#define	N_VERTS		(8)
#define	N_SPRTS		(N_PRIMS*N_VERTS)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define COL_R		(64)
#define COL_G		(64)
#define COL_B		(64)
#define ALPHA		(96)

#define	DEBRIS_NUM	(6)

#define	LIFE		(180)




extern void *NewCypherDebris( FVECTOR *pos, int kind, int parts );
extern void *NewCrashDebris( FVECTOR *pos );



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim_exp;

    FVECTOR	vec[N_SPRTS];
    float	rot[N_SPRTS];
    float	rot_add[N_SPRTS];
    float	rad[N_SPRTS];
    float	g[N_SPRTS];

    DG_PRIM2	*prim_smoke;

    int		cnt;

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;


    /* 爆発核 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim_exp ) )
    {
       return;
    }
    clock = work->prim_exp->buffer_clock;

    pos		= work->prim_exp->pos[clock];
    pos_pre	= work->prim_exp->pos[1-clock];
    uvrgbwh	= work->prim_exp->uvrgb[clock];
    uvrgbwh_pre = work->prim_exp->uvrgb[1-clock];
    vec 	= work->vec;

    for ( i = 0 ; i < N_SPRTS ; i++ ) {

	/* 位置更新 */
	_sceVu0AddVector( pos, pos_pre, vec );
	if ( work->cnt < 3 ) {
	    _sceVu0ScaleVector( vec, vec, 0.3f );
	}
	else {
	    _sceVu0ScaleVector( vec, vec, 0.98f );
	}

	pos++;
	pos_pre++;
	vec++;

	/* サイズ更新 */
	uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
	uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );
	work->rad[i] *= 1.006f;
	
	/* アルファ更新 */
	if ( uvrgbwh_pre->a > 0 ) {
	    uvrgbwh->a = (short) ( (float)uvrgbwh_pre->a * 0.95f );
	}
	else {
	    uvrgbwh->a = 0;
	}
	
	uvrgbwh++;

    }	
	


    /* 爆煙 */
    DG_SwitchBuffPrim2( work->prim_smoke );
    clock = work->prim_smoke->buffer_clock;

    pos		= work->prim_smoke->pos[clock];
    pos_pre	= work->prim_smoke->pos[1-clock];
    uvrgbwh	= work->prim_smoke->uvrgb[clock];
    uvrgbwh_pre = work->prim_smoke->uvrgb[1-clock];


    for ( i = 0 ; i < 20 ; i++ ) {

	if ( work->cnt < 8 ) {
	    uvrgbwh[i].w = uvrgbwh_pre[i].w * ( 1.f + 0.15f * (float)( 40 - (i/5) ) / 40.f );
	    uvrgbwh[i].h = uvrgbwh_pre[i].h * ( 1.f + 0.15f * (float)( 40 - (i/5) ) / 40.f );
	    uvrgbwh[i].a = uvrgbwh_pre[i].a;	
	}
	else {

	    uvrgbwh[i].w = uvrgbwh_pre[i].w * 1.003f;
	    uvrgbwh[i].h = uvrgbwh_pre[i].h * 1.003f;

	    if ( uvrgbwh_pre[i].a > 0 ) {

		if ( !(work->cnt%2) ) {
		    uvrgbwh[i].a = uvrgbwh_pre[i].a - 1;
		}
		else {
		    uvrgbwh[i].a = uvrgbwh_pre[i].a;
		}

		if ( work->cnt > 60 ) {
		    uvrgbwh[i].r = (short)( (float)uvrgbwh_pre[i].r * 0.98f );
		    uvrgbwh[i].g = (short)( (float)uvrgbwh_pre[i].g * 0.98f );
		    uvrgbwh[i].b = (short)( (float)uvrgbwh_pre[i].b * 0.98f );
		}
		else {
		    uvrgbwh[i].r = (short)( (float)uvrgbwh_pre[i].r * 0.98f + 96.0f * 0.02f );
		    uvrgbwh[i].g = (short)( (float)uvrgbwh_pre[i].g * 0.98f + 96.0f * 0.02f );
		    uvrgbwh[i].b = (short)( (float)uvrgbwh_pre[i].b * 0.98f + 96.0f * 0.02f );
		}
		
	    }
	    else {
		uvrgbwh[i].w = 0;
		uvrgbwh[i].h = 0;
		uvrgbwh[i].a = 0;
	    }

	}

    }
    

    if ( ++work->cnt >= LIFE ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim_exp ) work->prim_exp = OK_FreePrim2( work->prim_exp );
    if ( work->prim_smoke ) work->prim_smoke = OK_FreePrim2( work->prim_smoke );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR* center, float size )
{
	int			i;

	DG_PRIM2		*prim;

	DG_TEX			*tex;

	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float 			ftmp;
	float			fcos;
	float			fsin;
	float			angle;


	/* 爆発コア初期化 */
	prim = work->prim_exp = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		return 0;
	}
	prim->raise = 0;

	
	tex = DG_GetTexture( GV_StrCode( "bombgas1_alp" ) );
	
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	for ( i = 0 ; i < N_SPRTS ; i++ ) {

	    DG_COPY_VEC( pos, center );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    work->rot[i] = TPI*frnd();
	    work->rad[i] = size * 1.50f * 0.8f * ( 1.2f + frnd()*0.2f );
	    uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i] );
	    uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i] );

	    ftmp = size * 0.45f + size * 0.15f * frnd();
	    angle  = TPI * frnd();
	    fcos = vu0_Cos( angle );
	    fsin = vu0_Sin( angle );
	    angle  = TPI * frnd();
	    vec->vx = ftmp * fcos * vu0_Cos( angle ) * 1.50f;
	    vec->vy = ftmp * fsin * 1.50f;
	    vec->vz = ftmp * fcos * vu0_Sin( angle ) * 1.50f;
	    //if ( vec->vy > 0.f ) vec->vy *= 1.2f;

	    if ( ftmp < 0.f ) ftmp = -ftmp;
	    ftmp = ( size * 0.6f - ftmp ) / ( size * 0.6f );
	    uvrgbwh->r  = 110 + irnd()%18;
	    uvrgbwh->g  = 32 + 58 * ftmp;
	    uvrgbwh->b  = 32 * ftmp;
	    uvrgbwh->a  = ALPHA * ( 0.8f + frnd()*0.2f );

	    work->g[i] = 32.f + frnd() * 16.f;

	    work->rot_add[i] =-0.004f * fcos;

	    pos++;
	    uvrgbwh++;
	    vec++;

	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );



	/* 爆発煙 */
	prim = work->prim_smoke = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 20, 1 );
	if ( prim == NULL ) {
		return 0;
	}

	prim->raise = 0;

	tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < 20 ; i++ ) {

	    DG_COPY_VEC( pos, center );
	    pos->vx += size * frnd();
	    pos->vy += size * frnd();
	    pos->vz += size * frnd();

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = 128;
	    uvrgbwh->g  = 58 + irnd()%16;
	    uvrgbwh->b  = 0;
	    uvrgbwh->a  = 48 + irnd()%16;

	    ftmp  = size * ( 1.0f + frnd() * 0.2f );
	    angle = TPI * frnd();
	    uvrgbwh->w  = ftmp * vu0_Cos( angle );
	    uvrgbwh->h  = ftmp * vu0_Sin( angle );

	    pos++;
	    uvrgbwh++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          20 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          20 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 20 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 20 );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );


	return 1;

}


static int GetResources( Work *work, FVECTOR* center, float size, int flag )
{

    int 	i;
    
    work->cnt = 0;
    
    if ( !( InitPacket( work, center, size ) ) ) {
		return -1;
	}

    NewCrashDebris( center );

    for ( i = 0 ; i < DEBRIS_NUM ; i++ ) {
		NewCypherDebris( center, flag, (i/2) );
    }

    return 0;

}


void *NewCypherExplosion( FVECTOR* center, float size, int flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		if ( GetResources( work, center, size, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


