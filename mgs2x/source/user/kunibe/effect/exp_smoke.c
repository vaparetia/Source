//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    exp_smoke.c
    爆発
    2001/04/14 Yuuta Kunibe	
    $Id: exp_smoke.c,v 1.1.1.3 2002/11/19 11:44:38 Yoshizawa1 Exp $
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


#define COL_R	(64)
#define COL_G	(64)
#define COL_B	(64)

#define GLAY_R	(64)
#define ALPHA	(128)

#define N_PRIMS	(32)
#define	LIFE	(180)

//#define E3_VERSION

#ifdef E3_VERSION
#define INIT_RADIUS	(1600.0f)
#define MAX_SPEED	(1000.0f)
#else
#define INIT_RADIUS	(800.0f)
#define MAX_SPEED	(500.0f)
#endif

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	vec[N_PRIMS];
    float	rot[N_PRIMS];
    float	rot_add[N_PRIMS];
    float	rad[N_PRIMS];
    float	g[N_PRIMS];

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

    int			alpha_cnt;


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

    alpha_cnt = 0;

    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	_sceVu0AddVector( pos, pos_pre, vec );
	if ( work->cnt < 2 ) {
	    _sceVu0ScaleVector( vec, vec, 0.3f );
	}
	else {
	    _sceVu0ScaleVector( vec, vec, 0.92f );
	}

	pos++;
	pos_pre++;
	vec++;

		work->rot[i] += work->rot_add[i];
		if ( work->rot[i] >= 1.0f ) {
		    work->rot[i] = work->rot[i] - 1.0f;
		}
		else if ( work->rot[i] < 0.0f ) {
		    work->rot[i] = work->rot[i] + 1.0f;
		}

		uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
		uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );

		if ( uvrgbwh_pre->a >= 3 ) {
		    uvrgbwh->a = uvrgbwh_pre->a - 3;
		    if ( work->cnt > 10 ) {
			if ( uvrgbwh_pre->r > 0 ) {
			    /*uvrgbwh->g = uvrgbwh->r * uvrgbwh_pre->g / uvrgbwh_pre->r;
			    uvrgbwh->b = (u_short)( (float)uvrgbwh_pre->b * 0.98f );*/
			    uvrgbwh->g = (u_short)( (float)uvrgbwh_pre->g * 0.8f + work->g[i] * 0.2f );
			    uvrgbwh->b = (u_short)( (float)uvrgbwh_pre->b * 0.8f );
			}
			else {
			    uvrgbwh->r = 0;
			    uvrgbwh->g = 0;
			    uvrgbwh->b = 0;
			    alpha_cnt++;
			}
		    }
		    
		    /*if ( !(work->cnt%4) ) {
			if ( uvrgbwh_pre->g > 32 ) {
			    uvrgbwh->g = uvrgbwh_pre->g - 1;
			}
			else {
			    uvrgbwh->g = 32;
			}		    
			if ( uvrgbwh_pre->b > 0 ) {
			    uvrgbwh->b = uvrgbwh_pre->b - 1;
			}
			else {
			    uvrgbwh->b = 0;
			}
		    }
		    else {
			uvrgbwh->g = uvrgbwh_pre->g;
			uvrgbwh->b = uvrgbwh_pre->b;
		    }*/

		}
		else {
		    uvrgbwh->a = 0;
		    alpha_cnt++;
		}

		work->rad[i] *= 1.006f;

		uvrgbwh++;
    }
	

    if ( ++work->cnt >= LIFE || alpha_cnt >= N_PRIMS ) {
	//printf("cnt %d\n",work->cnt);
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR* center, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float 			ftmp;
	float			fcos;
	float			fsin;
	float			angle;

	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	
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

	    work->rot[i] = TPI*frnd();
	    work->rad[i] = INIT_RADIUS;
	    uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i] );
	    uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i] );

	    ftmp = MAX_SPEED * 0.7f + MAX_SPEED * 0.3f * frnd();
	    angle  = TPI * frnd();
	    fcos = vu0_Cos( angle );
	    fsin = vu0_Sin( angle );
	    angle  = TPI * frnd();
	    vec->vx = ftmp * fcos * vu0_Cos( angle );
	    vec->vy = ftmp * fsin;
	    vec->vz = ftmp * fcos * vu0_Sin( angle );
	    if ( vec->vy > 0.f ) vec->vy *= 1.2f;

	    if ( ftmp < 0.f ) ftmp = -ftmp;
	    ftmp = ( MAX_SPEED - ftmp ) / MAX_SPEED;
	    uvrgbwh->r  = 110 + irnd()%18;
	    uvrgbwh->g  = 32 + 58 * ftmp;
	    uvrgbwh->b  = 32 * ftmp;
	    uvrgbwh->a  = ALPHA * (0.8f + frnd()*0.2f);

	    work->g[i] = 32.f + frnd() * 16.f;

	    work->rot_add[i] =-0.004f * fcos;

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


static int GetResources( Work *work, FVECTOR* center )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->cnt = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				      N_PRIMS, 1 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas1_alp" ) );
    InitPacket( work, center, prim, tex );    

    return 0;

}


/*
  呼びだし関数
  FVECTOR* center : 煙発生点
  float    radius : 拡散半径	
                      人   -> 800.f～1000.f 
                    メタル -> 10000.f 
 */
void *NewExplosionSmoke( FVECTOR* center )
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


