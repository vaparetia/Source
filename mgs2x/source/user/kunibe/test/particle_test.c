//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    prm_test.c
    スプライト表示テスト
    2001/03/12 Yuuta Kunibe	
    $Id: particle_test.c,v 1.1.1.3 2002/11/19 11:44:55 Yoshizawa1 Exp $
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

#define COL_R	(128)
#define COL_G	(128)
#define COL_B	(128)
#define ALPHA	(64)

#define N_PRIMS	(100)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	(60)

typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int		map;

    DG_PRIM2	*prim;

    FVECTOR	center;
    FVECTOR	vec[N_PRIMS];    
    int 	life;

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

    /* スプライト中心座標取得 */
    work->center = GM_PlayerPosition;

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
    
    for( i=0; i<N_PRIMS; i++ ){
	    
	pos->vx = pos_pre->vx + vec->vx;
	pos->vy = pos_pre->vy + vec->vy;
	pos->vz = pos_pre->vz + vec->vz;
	pos->vw = 1.0f;

	if ( work->life < 30 ) {
	    uvrgbwh->a = uvrgbwh_pre->a * 0.95f;
	}

	vec->vx = vec->vx * 0.92f;
	vec->vz = vec->vz * 0.92f;

	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;
    }
    
    if( --work->life < 0 ){
	GV_DestroyActor( work );
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float			speed;
	float 			angle;
	FVECTOR			fvtmp;
	SVECTOR			svtmp;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	fvtmp.vy = 0.0f;
	svtmp.vx = 0;
	svtmp.vy = 0;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, &work->center );

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
	    uvrgbwh->a  = ALPHA;
	    
	    uvrgbwh->w  = 20.0f;
	    uvrgbwh->h  = 20.0f;

	    speed = 80.0f + frnd() * 80.0f;
	    angle = frnd() * TPI;
	    vec->vx = speed * vu0_Cos( angle );
	    vec->vz = speed * vu0_Sin( angle );
	    
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



static int GetResources( Work *work )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;

    work->life = GCL_GetOptionValue( 'l', -1 );
    work->center = GM_PlayerPosition;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );

    tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
    InitPacket( work, prim, tex );    
    
    if( work->life < 0 ) return -1;
    return 0;

}


void *NewParticleTest( int name, int map )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





