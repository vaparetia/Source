//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_light.c
  ハリアーライト
  2001/04/16 Yuuta Kunibe	
  $Id: harrier_light.c,v 1.1.1.3 2002/11/19 11:44:41 Yoshizawa1 Exp $
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
#define ALPHA	(128)

#define N_PRIMS	(1)

#define SIZE	(420)

#define	SHIFT	(200.0f)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	(60)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	*pos;

    int 	count;

} Work;



static void Act( Work *work )
{

    int	  		clock;
    FVECTOR  		*pos;
    FVECTOR		shift;
    DG_PRIM2_UVRGBWH	*uvrgbwh;

    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;
    pos = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];

    /* 位置更新 */
    *pos = *work->pos;		
    _sceVu0SubVector( &shift, (FVECTOR*)DG_Chanls[0].eye.m[3],pos );
    _sceVu0Normalize( &shift, &shift );
    _sceVu0ScaleVectorXYZ( &shift, &shift, SHIFT );
    _sceVu0AddVector( pos, pos, &shift);	


    if ( ++work->count >= DIRECT_TICK( 60 ) ) {
	work->count = 0;
    }

    if ( work->count < 10 ) {
	if ( work->count < 3 ) {
	    uvrgbwh->w = uvrgbwh->h = SIZE / 3 * work->count;
	}
	else if ( work->count > 6 ) {
	    uvrgbwh->w = uvrgbwh->h = SIZE / 3 * ( 9 - work->count );
	}
	else {
	    uvrgbwh->w = uvrgbwh->h = SIZE;
	}
	DG_VisiblePrim2( work->prim );	
    }
    else {
	uvrgbwh->w = 0;
	uvrgbwh->h = 0;
	DG_InvisiblePrim2( work->prim );
    }
    
    
    /*if( --work->life < 0 ){
	GV_DestroyActor( work );
    }*/

}


static void Die(Work *work )
{
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int color )
{

	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int			r,g,b,a;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	r = ( ( color >> 24 ) & 0xff );
	g = ( ( color >> 16 ) & 0xff );
	b = ( ( color >>  8 ) & 0xff );
	a = ( color & 0xff );


	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	DG_COPY_VEC( pos, work->pos );

	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = r;//COL_R;
	uvrgbwh->g  = g;//COL_G;
	uvrgbwh->b  = b;//COL_B;
	uvrgbwh->a  = a;//ALPHA;
	
	uvrgbwh->w  = SIZE;
	uvrgbwh->h  = SIZE;
	
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR *pos, int color )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;

    work->pos = pos;
    work->count = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
    if ( prim == NULL ) {
	return -1;
    }

    //    tex = DG_GetTexture( GV_StrCode( "svc_bonbori" ) );
    tex = DG_GetTexture( GV_StrCode( "ray_eye_bonbori_alp" ) );
    InitPacket( work, prim, tex, color );    
    
    return 0;
}


void *NewHarrierLight( FVECTOR *pos, int color )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, color ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





