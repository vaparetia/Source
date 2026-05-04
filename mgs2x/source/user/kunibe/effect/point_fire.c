//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    point_fire.c
    
    2001/04/25 Yuuta Kunibe	
    $Id: point_fire.c,v 1.1.1.3 2002/11/19 11:44:45 Yoshizawa1 Exp $
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


#define N_PRIMS	(8)

#define COLOR	(128)
#define ALPHA	(128)
#define SIZE	(100.0f)

#define	LIFE	(360)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define WIND_MAX (20.f)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR*	pos;
    FVECTOR	position;
    float	radius;

    FVECTOR	vec[N_PRIMS];
    float	rad[N_PRIMS];
    float	rot[N_PRIMS];
    float	rot_add[N_PRIMS];

    float	initsize;
    float	rising_spd;
    FVECTOR	wind;

    int  	id;
    int 	life;
    int		cnt;

} Work;


//void *NewPointFire( FVECTOR* pos );



/* アクト関数 */
static void Act( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR  		*vec;

    float		cos;
    int			alpha_count;
    

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
    vec		= work->vec;

    alpha_count = 0;

    /* スプライト更新 */
    for ( i = 0 ; i < N_PRIMS ; i++ ) {	

	if ( uvrgbwh_pre->a > 105 ) {

	    _sceVu0AddVector( pos, pos_pre, vec );
	    vec->vx = 12.5f * frnd();
	    vec->vz = 12.5f * frnd();

	    cos = vu0_Cos( work->rot[i] );
	    uvrgbwh->w = work->rad[i] * cos;
	    uvrgbwh->h = work->rad[i] * cos;

	    work->rad[i] *= 0.70f;
	    vec->vy += 1.0f;
	    uvrgbwh->a = uvrgbwh_pre->a - 1;

	    work->rot[i] += work->rot_add[i];
	    if ( work->rot[i] > PI ) work->rot[i] -= TPI;
	    else if ( work->rot[i] < -PI ) work->rot[i] += TPI;

	}
	else {
	    uvrgbwh->a = 0;
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;

	    alpha_count++;
	}

	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;
	
    }

    /*if ( ++work->cnt == 5 ) {
	NewPointFire( &GM_PlayerPosition );
    }*/
    
    if ( alpha_count >= N_PRIMS ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float 			ftmp;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, work->pos );
	    pos->vx += frnd()*work->radius/2.f;
	    pos->vz += frnd()*work->radius/2.f;

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = 128;
	    uvrgbwh->g  = 24 + irnd()%24;
	    uvrgbwh->b  = 8;
	    uvrgbwh->a  = 106 + irnd()%3;

	    ftmp = ( 0.5f + frnd() * 0.50f );
	    work->rad[i] = work->radius * ( 0.5f + ftmp );
	    work->rot[i] = PI/2.f * frnd();
	    uvrgbwh->w = work->rad[i] * vu0_Cos( work->rot[i] );
	    uvrgbwh->h = work->rad[i] * vu0_Sin( work->rot[i] );

	    DG_COPY_VEC( vec, &DG_ZeroVector );
	    vec->vx = 20.f * frnd();
	    vec->vy = 20.f * ( 1.0f - ftmp ) / 1.0f + 10.f; 
	    vec->vz = 20.f * frnd();
	    /*vec->vx = 100.f * frnd();
	    vec->vy = 150.f * frnd() + 200.f; 
	    vec->vz = 100.f * frnd();*/

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



static int GetResources( Work *work, FVECTOR* pos )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->life = LIFE;
    work->cnt  = 0;

    work->pos      = pos;
    work->position = *pos;
    work->radius   = 100.f;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 8, 1 );
	if ( prim == NULL ) {
		return -1;
	}

    //tex = DG_GetTexture( GV_StrCode( "chi01_msk" ) );
    tex = DG_GetTexture( GV_StrCode( "blood_2bw_msk" ) );
    InitPacket( work, prim, tex );    

    return 0;

}


/*
  NewRisingSmoke
      FVECTOR* pos : 発生位置
*/
void *NewPointFire( FVECTOR* pos )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



