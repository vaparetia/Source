//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    blood_smoke.c
    血煙
    2001/07/28 Yuuta Kunibe	
    $Id: blood_smoke.c,v 1.1.1.3 2002/11/19 11:44:33 Yoshizawa1 Exp $
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


extern int BP_FRAMES_PER_SEC();

#define N_PRIMS		(2)
#define	N_VERTS		(16)
#define	N_SPRTS		(N_PRIMS*N_VERTS)

#define COL_R		(255)
#ifdef ENGLSH
#define COL_G		(230)
#define COL_B		(230)
#else
#define COL_G		(243)//(230)
#define COL_B		(243)//(230)
#endif

#define ALPHA		(6)

#define	LIFE		(240)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define	SMOKE_TEX	( GV_StrCode( "w00_sky_add_alp" ) )



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    float	rot[N_SPRTS];
    float	rad[N_SPRTS];
    int		alpha[N_SPRTS];

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

    int 	alpha_cnt;
    int buffSwitch;




    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_FULL
    buffSwitch = DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[buffSwitch ^ clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[buffSwitch ^ clock];



    alpha_cnt = 0;
    
    /*----------------*/
    /* スプライト更新 */
    /*----------------*/
    for ( i = 0 ; i < N_SPRTS ; i++ ) {

	if ( work->cnt < BP_FRAMES_PER_SEC() ) {					//60
	    uvrgbwh->w = uvrgbwh_pre->w + 10;
	    uvrgbwh->h = uvrgbwh_pre->h + 10;
	    uvrgbwh->a = work->alpha[i] * work->cnt / BP_FRAMES_PER_SEC();	// 60
	}
	else {
	
	    if ( uvrgbwh_pre->a > 0 ) {
		uvrgbwh->w = uvrgbwh_pre->w + 10;
		uvrgbwh->h = uvrgbwh_pre->h + 10;
		if ( i%8 == work->cnt%8 ) {
		    uvrgbwh->a = uvrgbwh_pre->a - 1;
		}
		else {
		    uvrgbwh->a = uvrgbwh_pre->a;
		}
	    }
	    else {
		uvrgbwh->w = 0;
		uvrgbwh->h = 0;
		uvrgbwh->a = 0;
		alpha_cnt++;
	    }

	}

	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;

    }

    work->cnt++;


    if ( alpha_cnt >= N_SPRTS ) {
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
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;


	for ( i = 0 ; i < N_SPRTS ; i++ ) {

	    DG_COPY_VEC( pos, center );
	    pos->vx += frnd()*1000.0f;
	    pos->vy += frnd()*1000.0f;
	    pos->vz += frnd()*1000.0f;
	    pos->vw = 1.0f;

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = 1;//~COL_R;
	    uvrgbwh->g  = ~COL_G;
	    uvrgbwh->b  = ~COL_B;
	    uvrgbwh->a  = 0;
	    work->alpha[i] = ALPHA + irnd()%8;

	    work->rot[i] = frnd();
	    work->rad[i] = 800.0f + frnd()*400.0f;
	    uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*PI );
	    uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*PI );

	    pos++;
	    uvrgbwh++;

	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}




static int GetResources( Work *work, FVECTOR* center )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->cnt = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
		return -1;
    }

    tex = DG_GetTexture( SMOKE_TEX );

    InitPacket( work, center, prim, tex );    

    return 0;

}






void *NewBloodSmoke( FVECTOR* center )
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



